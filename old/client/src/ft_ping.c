/*
 * ft_ping.c - minimal ping-like program
 *
 * Usage: ./ft_ping [-v] <hostname|IPv4>
 *
 * - Implements basic ICMP Echo Request/Reply using raw sockets (IPv4 only).
 * - Handles -v (verbose) and -? (help).
 * - Must be run as root (raw socket).
 *
 * Note: This is an educational minimal implementation. It is not a full
 * replacement for system ping and does not implement all edge cases.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <assert.h>
#include <stdint.h>
#include <time.h>

#define PACKET_SIZE 64
#define DEFAULT_INTERVAL 1
#define RECV_TIMEOUT_SEC 1

/* Global state for SIGINT printing */
static volatile sig_atomic_t running = 1;

/* Statistics */
static unsigned long transmitted = 0;
static unsigned long received_packets = 0;
static double rtt_min = 1e9, rtt_max = 0, rtt_sum = 0;

/* Options */
static int opt_verbose = 0;

/* Program id and socket */
static pid_t prog_pid;
static int sockfd = -1;

/* Target address info */
static struct sockaddr_in target_addr;
static char target_ip_str[INET_ADDRSTRLEN];
static char target_name[256] = {0};

/* Sequence number */
static uint16_t seq_no = 0;

/* Utility: print usage */
static void usage(const char* prog) {
  fprintf(stderr,
          "Usage: %s [-v] <hostname|IPv4>\n"
          "  -v    verbose (show errors / detailed info)\n"
          "  -?    this help\n",
          prog);
}

/* SIGINT handler */
static void sigint_handler(int signo) {
  (void)signo;
  running = 0;
}

/* Compute ICMP checksum (RFC 1071) */
static uint16_t icmp_checksum(const void* buf, int len) {
  const uint16_t* data = buf;
  uint32_t sum = 0;

  while (len > 1) {
    sum += *data++;
    len -= 2;
  }

  if (len == 1) {
    uint8_t last = *(const uint8_t*)data;
    sum += (uint16_t)last << 8; /* pad high byte (network byte order) */
  }

  /* fold 32-bit sum to 16 bits */
  while (sum >> 16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  return (uint16_t)(~sum);
}

/* Time difference in milliseconds */
static double time_diff_ms(const struct timeval* start,
                           const struct timeval* end) {
  double s = (double)(end->tv_sec - start->tv_sec) * 1000.0;
  double us = (double)(end->tv_usec - start->tv_usec) / 1000.0;
  return s + us;
}

/* Build and send an ICMP Echo Request with current seq_no */
static int send_echo_request(int sock) {
  uint8_t packet[PACKET_SIZE];
  memset(packet, 0, sizeof(packet));

  struct icmphdr* icmp = (struct icmphdr*)packet;
  icmp->type = ICMP_ECHO;
  icmp->code = 0;
  icmp->un.echo.id = htons((uint16_t)prog_pid);
  icmp->un.echo.sequence = htons(seq_no);

  /* payload: store send timestamp for RTT calculation */
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) {
    perror("gettimeofday");
    return -1;
  }
  size_t header_len = sizeof(struct icmphdr);
  size_t payload_len = sizeof(struct timeval);
  if (header_len + payload_len > sizeof(packet))
    payload_len = sizeof(packet) - header_len;
  memcpy(packet + header_len, &tv, payload_len);

  /* checksum */
  icmp->checksum = 0;
  icmp->checksum = icmp_checksum(packet, header_len + payload_len);

  ssize_t sent = sendto(sock, packet, header_len + payload_len, 0,
                        (struct sockaddr*)&target_addr, sizeof(target_addr));
  if (sent < 0) {
    if (opt_verbose)
      perror("sendto");
    return -1;
  }

  transmitted++;
  return 0;
}

/* Receive loop: wait for ICMP reply and process it */
static int receive_one(int sock, int timeout_sec) {
  uint8_t buf[1500];
  struct sockaddr_in from;
  socklen_t fromlen = sizeof(from);

  struct timeval tv;
  tv.tv_sec = timeout_sec;
  tv.tv_usec = 0;

  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sock, &readfds);

  int sel = select(sock + 1, &readfds, NULL, NULL, &tv);
  if (sel < 0) {
    if (errno == EINTR)
      return -1;
    perror("select");
    return -1;
  } else if (sel == 0) {
    /* timeout */
    return 0;
  }

  ssize_t recvd =
      recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&from, &fromlen);
  if (recvd < 0) {
    if (errno == EINTR)
      return -1;
    if (opt_verbose)
      perror("recvfrom");
    return -1;
  }

  /* Parse IP header first */
  if (recvd < (ssize_t)sizeof(struct iphdr)) {
    if (opt_verbose)
      fprintf(stderr, "Received too small packet (%zd bytes)\n", recvd);
    return -1;
  }

  struct iphdr* iph = (struct iphdr*)buf;
  size_t ip_header_len = iph->ihl * 4;
  if (recvd < (ssize_t)(ip_header_len + sizeof(struct icmphdr))) {
    if (opt_verbose)
      fprintf(stderr, "Packet too short for ICMP (recv %zd iphdr %zu)\n", recvd,
              ip_header_len);
    return -1;
  }

  struct icmphdr* icmp = (struct icmphdr*)(buf + ip_header_len);
  int icmp_len = recvd - ip_header_len;

  /* Source IP string */
  char from_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &from.sin_addr, from_ip, sizeof(from_ip));

  /* If ICMP Echo Reply */
  if (icmp->type == ICMP_ECHOREPLY) {
    uint16_t id = ntohs(icmp->un.echo.id);
    uint16_t seq = ntohs(icmp->un.echo.sequence);
    if ((pid_t)id != prog_pid) {
      /* not for us */
      if (opt_verbose)
        fprintf(stderr, "Ignored echo reply for pid %u (ours %u)\n", id,
                prog_pid);
      return -1;
    }

    /* Extract send time from payload if present */
    struct timeval send_tv, recv_tv;
    memset(&send_tv, 0, sizeof(send_tv));
    if (icmp_len >= (int)(sizeof(struct icmphdr) + sizeof(struct timeval))) {
      memcpy(&send_tv, (uint8_t*)icmp + sizeof(struct icmphdr),
             sizeof(struct timeval));
    } else {
      /* fallback: unknown send time */
      send_tv.tv_sec = 0;
      send_tv.tv_usec = 0;
    }

    if (gettimeofday(&recv_tv, NULL) < 0) {
      perror("gettimeofday");
      return -1;
    }

    double rtt_ms = 0;
    if (send_tv.tv_sec != 0 || send_tv.tv_usec != 0)
      rtt_ms = time_diff_ms(&send_tv, &recv_tv);

    received_packets++;
    rtt_sum += rtt_ms;
    if (rtt_ms < rtt_min)
      rtt_min = rtt_ms;
    if (rtt_ms > rtt_max)
      rtt_max = rtt_ms;

    printf("%zd bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
           (ssize_t)(icmp_len - sizeof(struct icmphdr)), from_ip, seq, iph->ttl,
           rtt_ms);
    fflush(stdout);
    return 1;
  } else {
    /* ICMP error or other type */
    if (opt_verbose) {
      printf("Received ICMP type=%d code=%d from %s (ttl=%u)\n", icmp->type,
             icmp->code, from_ip, iph->ttl);
      fflush(stdout);
    }
    return -1;
  }
}

/* Print summary statistics */
static void print_stats(void) {
  printf("\n--- %s ping statistics ---\n",
         target_name[0] ? target_name : target_ip_str);
  printf("%lu packets transmitted, %lu received, ", transmitted,
         received_packets);
  unsigned long lost = transmitted - received_packets;
  double loss = transmitted ? (lost * 100.0 / transmitted) : 0.0;
  printf("%.1f%% packet loss\n", loss);
  if (received_packets > 0) {
    double avg = rtt_sum / received_packets;
    printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n", rtt_min, avg, rtt_max);
  }
}

/* Resolve hostname to IPv4 sockaddr_in */
static int resolve_target(const char* host) {
  struct addrinfo hints, *res = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; /* IPv4 only */
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;

  int rc = getaddrinfo(host, NULL, &hints, &res);
  if (rc != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    return -1;
  }
  assert(res);

  struct sockaddr_in* sin = (struct sockaddr_in*)res->ai_addr;
  memset(&target_addr, 0, sizeof(target_addr));
  target_addr.sin_family = AF_INET;
  target_addr.sin_addr = sin->sin_addr;
  inet_ntop(AF_INET, &sin->sin_addr, target_ip_str, sizeof(target_ip_str));
  strncpy(target_name, host, sizeof(target_name) - 1);

  freeaddrinfo(res);
  return 0;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  /* Parse options */
  int arg_index = 1;
  while (arg_index < argc && argv[arg_index][0] == '-') {
    if (strcmp(argv[arg_index], "-v") == 0) {
      opt_verbose = 1;
    } else if (strcmp(argv[arg_index], "-?") == 0) {
      usage(argv[0]);
      return EXIT_SUCCESS;
    } else {
      fprintf(stderr, "Unknown option: %s\n", argv[arg_index]);
      usage(argv[0]);
      return EXIT_FAILURE;
    }
    arg_index++;
  }

  if (arg_index >= argc) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  const char* target = argv[arg_index];

  /* Setup global variables */
  prog_pid = getpid();

  if (resolve_target(target) < 0) {
    return EXIT_FAILURE;
  }

  /* Create raw socket */
  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sockfd < 0) {
    perror("socket");
    fprintf(stderr, "Note: raw sockets require root privileges (try sudo).\n");
    return EXIT_FAILURE;
  }

  /* Install SIGINT handler */
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigint_handler;
  sigaction(SIGINT, &sa, NULL);

  printf("PING %s (%s): %d data bytes\n", target, target_ip_str,
         (int)(PACKET_SIZE - sizeof(struct icmphdr)));
  fflush(stdout);

  /* Main loop: send 1 echo request per second until SIGINT */
  while (running) {
    if (send_echo_request(sockfd) == 0) {
      /* increment sequence after sending */
      seq_no++;
    }

    /* Wait for a reply with timeout */
    int got = receive_one(sockfd, RECV_TIMEOUT_SEC);
    if (got == 0) {
      printf("Request timeout for icmp_seq %u\n", seq_no);
      fflush(stdout);
    }
    /* Sleep to match 1-second spacing (roughly) */
    sleep(DEFAULT_INTERVAL);
  }

  /* Print final stats */
  print_stats();

  if (sockfd >= 0)
    close(sockfd);
  return EXIT_SUCCESS;
}
