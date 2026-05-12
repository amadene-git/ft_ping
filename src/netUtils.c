#include <netUtils.h>
#include <timeUtils.h>
#include <utils.h>

typedef struct s_socketCLient {
} t_socketCLient;

void initializeRawSocket(const char* hostname, t_ping* ping) {
  ping->rawSocket = galloc(sizeof(t_rawSocket), ping);
  if ((ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
    exitProgram("socket() failed", errno, true, ping);
  }
  struct timeval recvTimeout = {FT_PING_RECV_TIMEOUT_SEC, 0};
  if (setsockopt(ping->sockfd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout)) == -1) {
    exitProgram("setsockopt() failed", errno, true, ping);
  }
  ping->rawSocket->_hostname = ft_strdup(hostname, ping);

  resolveDNS(ping);
  ping->rawSocket->_sockAddr.sin_family = AF_INET;
  ping->rawSocket->_socklen = sizeof(struct sockaddr_in);
}

int resolveDNS(t_ping* ping) {
  struct addrinfo hints;
  struct addrinfo* result = NULL;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;

  int ret = getaddrinfo(ping->rawSocket->_hostname, NULL, &hints, &result);
  if (ret != 0 || result == NULL) {
    char buffer[100] = {0};
    sprintf(buffer, "ft_ping: unknown host");
    exitProgram(buffer, 2, false, ping);
  }
  ping->rawSocket->_sockAddr = *(struct sockaddr_in*)result->ai_addr;
  freeaddrinfo(result);

  char str_ip[16] = {0};
  inet_ntop(AF_INET, &ping->rawSocket->_sockAddr.sin_addr.s_addr, &str_ip[0], 100);
  strncpy(&ping->rawSocket->_ipAddress[0], &str_ip[0], 15);

  return 0;
}

void* buildIcmpHeader(void* hdrPtr, uint16_t seqnum) {
  struct icmphdr* header = (struct icmphdr*)hdrPtr;

  header->type = ICMP_ECHO;
  header->code = 0;

  header->checksum = 0;

  header->un.echo.id = htons((uint16_t)getpid());
  header->un.echo.sequence = htons(seqnum);

  return hdrPtr + sizeof(struct icmphdr);
}

void icmpChecksum(const void* packet, int len) {
  const uint16_t* data = packet;
  uint32_t sum = 0;

  while (len > 1) {
    sum += *data++;
    len -= 2;
  }

  if (len == 1) {  // padding
    uint8_t last = *(const uint8_t*)data;
    sum += (uint16_t)last << 8;
  }

  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  struct icmphdr* header = (struct icmphdr*)packet;
  header->checksum = (uint16_t)(~sum);
}

void sendPacket(t_ping* ping) {
  bzero(ping->packet, ping->packetSize);
  buildIcmpHeader(ping->packet, ping->seqnum);
  icmpChecksum(ping->packet, ping->packetSize);

  initializeCurrentRtt(ping);

  if ((size_t)sendto(ping->sockfd,
                     ping->packet,
                     ping->packetSize,
                     MSG_CONFIRM,
                     (struct sockaddr*)(&ping->rawSocket->_sockAddr),
                     ping->rawSocket->_socklen) != ping->packetSize) {
    exitProgram("sendTo() failed", errno, true, ping);
  }

  ++ping->stats.nbSend;
}

static int isExpectedEchoReply(t_ping* ping,
                               const uint8_t* recvBuffer,
                               ssize_t nbBytesRecv,
                               const struct sockaddr_in* recvAddr,
                               uint8_t* ttl) {
  if ((size_t)nbBytesRecv < sizeof(struct iphdr)) {
    return 0;
  }

  const struct iphdr* ipHeader = (const struct iphdr*)recvBuffer;
  size_t ipHeaderLen = ipHeader->ihl * 4;
  if (ipHeader->version != 4 || ipHeaderLen < sizeof(struct iphdr) ||
      (size_t)nbBytesRecv < ipHeaderLen + sizeof(struct icmphdr)) {
    return 0;
  }
  if ((size_t)nbBytesRecv < ipHeaderLen + ping->packetSize) {
    return 0;
  }

  if (recvAddr->sin_addr.s_addr != ping->rawSocket->_sockAddr.sin_addr.s_addr ||
      ipHeader->saddr != ping->rawSocket->_sockAddr.sin_addr.s_addr) {
    return 0;
  }

  const struct icmphdr* icmpHeader = (const struct icmphdr*)(recvBuffer + ipHeaderLen);
  if (icmpHeader->type != ICMP_ECHOREPLY || icmpHeader->code != 0) {
    return 0;
  }

  if (ntohs(icmpHeader->un.echo.id) != (uint16_t)getpid()) {
    return 0;
  }
  if (ntohs(icmpHeader->un.echo.sequence) != (uint16_t)ping->seqnum) {
    return 0;
  }

  *ttl = ipHeader->ttl;
  return 1;
}

static void dumpVerboseIcmpError(const struct iphdr* oIp, const struct icmphdr* oIcmp) {
  const uint16_t* p = (const uint16_t*)oIp;
  ft_printf("IP Hdr Dump:\n ");
  for (int i = 0; i < 10; i++) ft_printf(" %04x", ntohs(p[i]));
  ft_printf(
      "\n"
      "Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src       Dst     Data\n");
  ft_printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %u.%u.%u.%u  %u.%u.%u.%u\n",
            oIp->version,
            oIp->ihl,
            oIp->tos,
            ntohs(oIp->tot_len),
            ntohs(oIp->id),
            (ntohs(oIp->frag_off) >> 13) & 0x7,
            ntohs(oIp->frag_off) & 0x1FFF,
            oIp->ttl,
            oIp->protocol,
            ntohs(oIp->check),
            (ntohl(oIp->saddr) >> 24) & 0xff,
            (ntohl(oIp->saddr) >> 16) & 0xff,
            (ntohl(oIp->saddr) >> 8) & 0xff,
            ntohl(oIp->saddr) & 0xff,
            (ntohl(oIp->daddr) >> 24) & 0xff,
            (ntohl(oIp->daddr) >> 16) & 0xff,
            (ntohl(oIp->daddr) >> 8) & 0xff,
            ntohl(oIp->daddr) & 0xff);
  ft_printf("ICMP: type %u, code %u, size %lu, id 0x%04x, seq 0x%04x\n",
            oIcmp->type,
            oIcmp->code,
            sizeof(struct icmphdr) + 56UL,
            ntohs(oIcmp->un.echo.id),
            ntohs(oIcmp->un.echo.sequence));
}

static const char* icmpErrorString(uint8_t type, uint8_t code) {
  static char fallback[36];
  static const char* unreach[16] = {
      "Destination Net Unreachable",       // 0
      "Destination Host Unreachable",      // 1
      "Destination Protocol Unreachable",  // 2
      "Destination Port Unreachable",      // 3
      "Fragmentation needed and DF set",   // 4
      "Source Route Failed",               // 5
      "Destination Net Unknown",           // 6
      "Destination Host Unknown",          // 7
      "Source Host Isolated",              // 8
      NULL,
      NULL,
      NULL,
      NULL,               // 9–12
      "Packet Filtered",  // 13
      NULL,
      NULL,  // 14–15
  };

  static const char* redirect[] = {
      "Redirect Network",
      "Redirect Host",
      "Redirect Type of Service and Network",
      "Redirect Type of Service and Host",
  };

  if (type == ICMP_DEST_UNREACH) {
    if (code < 16 && unreach[code]) {
      return unreach[code];
    }
    snprintf(fallback, sizeof(fallback), "Dest Unreachable, Unknown Code: %u", code);
    return fallback;
  }

  if (type == ICMP_REDIRECT) {
    if (code < 4) {
      return redirect[code];
    }
    snprintf(fallback, sizeof(fallback), "Redirect, Unknown Code: %u", code);
    return fallback;
  }
  if (type == ICMP_TIME_EXCEEDED) {
    return code ? "Frag reassembly time exceeded" : "Time to live exceeded";
  }
  if (type == ICMP_PARAMETERPROB) {
    return "Parameter problem";
  }
  if (type == 9) {
    return "Router Advertisement";
  }
  if (type == 10) {
    return "Router Discovery";
  }

  snprintf(fallback, sizeof(fallback), "Bad ICMP type: %u", type);
  return fallback;
}

static int printIcmpErrorIfForUs(t_ping* ping, const uint8_t* buf, ssize_t n) {
  const struct iphdr* ip = (const struct iphdr*)buf;
  size_t ipLen = ip->ihl * 4;
  const struct icmphdr* icmp = (const struct icmphdr*)(buf + ipLen);

  if ((size_t)n < ipLen + 8 + sizeof(struct iphdr) + 8) {
    return 0;
  }

  const struct iphdr* oIp = (const struct iphdr*)((uint8_t*)icmp + 8);
  const struct icmphdr* oIcmp = (const struct icmphdr*)((uint8_t*)oIp + oIp->ihl * 4);

  if (icmp->type == ICMP_ECHOREPLY || icmp->type == ICMP_ECHO) {
    return 0;
  }

  char src[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &ip->saddr, src, sizeof(src));
  ft_printf("%zd bytes from %s: %s\n", (ssize_t)(n - (ssize_t)ipLen), src, icmpErrorString(icmp->type, icmp->code));

  if (ping->verbose) dumpVerboseIcmpError(oIp, oIcmp);
  return 1;
}

ssize_t receivePacket(t_ping* ping, uint8_t* ttl) {
  uint8_t buf[IP_MAXPACKET];
  while (1) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    ssize_t n = recvfrom(ping->sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
    if (n == 0) {
      exitProgram("Receive no ping reply", EXIT_FAILURE, false, ping);
    }
    if (n == -1) {
      return (errno == EAGAIN || errno == EWOULDBLOCK) ? 0 : -1;
    }

    if (isExpectedEchoReply(ping, buf, n, &addr, ttl)) {
      size_t ipLen = ((struct iphdr*)buf)->ihl * 4;
      computeCurrentRtt(ping);
      updateStats(&ping->stats);
      return n - (ssize_t)ipLen;
    }
    printIcmpErrorIfForUs(ping, buf, n);
  }
}
