#include <netUtils.h>
#include <timeUtils.h>
#include <utils.h>

// std
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exitProgram(const char* message, int code, bool hasErrno, t_ping* ping) {
  if (code != EXIT_SUCCESS) {
    dprintf(2, "%s", message);
    if (hasErrno) dprintf(2, " (errno: %s)", strerror(errno));
    dprintf(2, "\n");
  }
  if (ping->sockfd != -1) {
    close(ping->sockfd);
  }
  freeGarbage(ping);
  exit(code);
}

int ft_printf(const char* format, ...) {
  char buffer[4096];
  int len;
  va_list args;

  va_start(args, format);
  len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (len < 0) return (-1);

  write(1, buffer, len);

  return (len);
}

void printFirstLog(t_ping* ping) {
  static const size_t ICMP_HEADER_SIZE = sizeof(struct icmphdr);

  ft_printf("FT_PING %s (%s): %zu data bytes\n",
            ping->rawSocket->_hostname,
            &ping->rawSocket->_ipAddress[0],
            ping->packetSize - ICMP_HEADER_SIZE);
}

void printLog(t_ping* ping, ssize_t nbBytesRecv, uint8_t ttl) {
  t_microsec rtt = timevalToUs(((t_RTT*)(*ping->stats.rtts)->data)->result);

  ft_printf("%zd bytes from %s: icmp_seq=%ld ttl=%hu time=%lu.%03lu ms\n",
            nbBytesRecv,
            ping->rawSocket->_ipAddress,
            ping->seqnum++,
            ttl,
            rtt / 1000,
            rtt % 1000);
}

void printStats(t_ping* ping) {
  ft_printf("\n--- %s ping statistics ---\n", ping->rawSocket->_hostname);
  ft_printf("%lu packets transmitted, ", ping->stats.nbSend);
  ft_printf("%lu received, ", ping->stats.nbRecv);
  ft_printf("%lu%% packet loss, ", computeLossPercent(ping->stats));
  ft_printf("time %lums\n", getProgramDuration(&ping->stats.progDuration, ping));
  if (ping->stats.nbRecv == 0 || *ping->stats.rtts == NULL) {
    return;
  }
  ft_printf("round-trip min/avg/max/mdev = ");

  t_microsec min = getMinRtt(*ping->stats.rtts);
  ft_printf("%lu.%lu/", min / 1000, min % 1000);

  t_rtt_stats meanAndDev = welfordAlgo(*ping->stats.rtts);
  ft_printf("%lu.%lu/", meanAndDev.avg / 1000, meanAndDev.avg % 1000);

  t_microsec max = getMaxRtt(*ping->stats.rtts);
  ft_printf("%lu.%lu/", max / 1000, max % 1000);

  ft_printf("%lu.%lums\n", meanAndDev.mdev / 1000, meanAndDev.mdev % 1000);
}

char* ft_strdup(const char* s, t_ping* ping) {
  size_t size = strlen(s) + 1;
  char* ret = galloc(size, ping);
  while (*s) {
    *ret++ = *s++;
  }
  *ret = 0;
  return (ret - (size - 1));
}

static void printErrorNoHost() {
  fprintf(stderr, "ft_ping: missing host operand\n");
  fprintf(stderr, "Try 'ping --help' for more information.\n");
}

static void printErrorInvalidOption(const char* option) {
  fprintf(stderr, "ft_ping: invalid option -- '%s'\n", option + 1);
  fprintf(stderr, "Try 'ping --help' for more information.\n");
}

static void printErrorUnrecognizedOption(const char* option) {
  fprintf(stderr, "ft_ping: unrecognized option '%s'\n", option);
  fprintf(stderr, "Try 'ping --help' for more information.\n");
}

static void printErrorTooManyHost(const char* host) {
  fprintf(stderr, "ft_ping: too many host '%s'\n", host);
  fprintf(stderr, "Try 'ping --help' for more information.\n");
}

static void printHelp() {
  printf("Usage:\n");
  printf("\t./ft_ping [OPTION...] HOST ...\n\n");

  printf("Small project for 42 School that recreates the ping command.\n");
  printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");

  printf(" Options:\n");
  printf("  -?, --help                 give this help list\n");
  printf("  -v, --verbose              verbose output\n");
}

const char* parseCommandLine(const int ac, const char** av, t_ping* ping) {
  if (ac < 2) {
    printErrorNoHost();
    exit(64);
  }

  const char* hostname = av[1];

  if (ac == 2 && (!strcmp(av[1], "-?") || !strcmp(av[1], "--help"))) {
    printHelp();
    exit(0);
  } else if (ac == 3) {
    if (!strcmp(av[1], "-v") || !strcmp(av[1], "--verbose")) {
      ping->verbose = true;
    } else if (!strcmp(av[1], "--")) {
      // do nothing
    } else if (!strncmp(av[1], "--", 2)) {
      printErrorUnrecognizedOption(av[1]);
      exit(64);
    } else if ((!strncmp(av[1], "-", 1))) {
      printErrorInvalidOption(av[1]);
      exit(64);
    } else {
      printErrorTooManyHost(av[2]);
      exit(64);
    }
    hostname = av[2];
  } else if (ac == 2) {
    if (!strcmp(av[1], "--")) {
      printErrorNoHost();
      exit(64);
    } else if (!strncmp(av[1], "--", 2)) {
      printErrorUnrecognizedOption(av[1]);
      exit(64);

    } else if (strlen(av[1]) > 1 && (!strncmp(av[1], "-", 1))) {
      printErrorInvalidOption(av[1]);
      exit(64);
    }
  }

  return hostname;
}
