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
    sprintf(buffer, "ft_ping: %s: Nom ou service inconnu", ping->rawSocket->_hostname);
    exitProgram(buffer, 2, false, ping);
  }
  ping->rawSocket->_sockAddr = *(struct sockaddr_in*)result->ai_addr;
  free(result);

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

  while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);

  struct icmphdr* header = (struct icmphdr*)packet;
  header->checksum = (uint16_t)(~sum);
}

void sendPacket(t_ping* ping) {
  bzero(ping->packet, ping->packetSize);
  buildIcmpHeader(ping->packet, ping->seqnum);
  icmpChecksum(ping->packet, ping->packetSize);

  t_RTT* rtt = galloc(sizeof(t_RTT), ping);
  listPushFront(ping->stats.rtts, listNewElem(rtt, ping), ping);

  *rtt = initRTT(ping);
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

ssize_t receivePacket(t_ping* ping, uint8_t* ttl) {
  uint8_t recvBuffer[IP_MAXPACKET];

  while (1) {
    struct sockaddr_in recvAddr;
    socklen_t recvLen = sizeof(recvAddr);
    bzero(recvBuffer, sizeof(recvBuffer));
    bzero(&recvAddr, sizeof(recvAddr));

    ssize_t nbBytesRecv =
        recvfrom(ping->sockfd, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr*)(&recvAddr), &recvLen);
    if (nbBytesRecv == 0) {
      exitProgram("Receive no ping reply", EXIT_FAILURE, false, ping);
    }
    if (nbBytesRecv == -1) {
      return -1;
    }

    if (isExpectedEchoReply(ping, recvBuffer, nbBytesRecv, &recvAddr, ttl)) {
      computeRTT((t_RTT*)((*ping->stats.rtts)->data), ping);
      ++ping->stats.nbRecv;
      return nbBytesRecv;
    }
  }
}
