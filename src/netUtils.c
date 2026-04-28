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

void* buildIcmpHeader(void* hdrPtr) {
  struct icmphdr* header = (struct icmphdr*)hdrPtr;
  static uint16_t seqno = 0;

  header->type = ICMP_ECHO;
  header->code = 0;  // network unreachable
  // header->code = 1;// host unreachable
  // header->code = 3;// port unreachable

  header->checksum = 0;

  header->un.echo.id = getpid();
  header->un.echo.sequence = seqno++;

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
  buildIcmpHeader(ping->packet);
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

ssize_t receivePacket(t_ping* ping, uint8_t* ttl) {
  char recvBuffer[ping->packetSize];
  bzero(recvBuffer, ping->packetSize);
  ssize_t nbBytesRecv = recvfrom(ping->sockfd,
                                 recvBuffer,
                                 ping->packetSize,
                                 0,
                                 (struct sockaddr*)(&ping->rawSocket->_sockAddr),
                                 &ping->rawSocket->_socklen);
  computeRTT((t_RTT*)((*ping->stats.rtts)->data), ping);
  if (nbBytesRecv == 0) {
    exitProgram("Receive no ping reply", EXIT_FAILURE, false, ping);
  }
  if (nbBytesRecv == -1) {
    return -1;
  }

  ++ping->stats.nbRecv;
  *ttl = ((struct iphdr*)recvBuffer)->ttl;

  return nbBytesRecv;
}