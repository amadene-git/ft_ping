#include <netUtils.h>

t_rawSocket* initializeRawSocket(const char* host) {
  t_rawSocket* rawSocket = NULL;
  if ((rawSocket = malloc(sizeof(t_rawSocket))) == NULL) {
    exitProgram("malloc() failed", errno, true);
  }

  if ((rawSocket->_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
    exitProgram("socket() failed", errno, true);
  }

  resolveDNS(host, rawSocket);
  rawSocket->_sockAddr.sin_family = AF_INET;
  rawSocket->_socklen = sizeof(struct sockaddr_in);
  rawSocket->_hostname = strdup(host);

  //   if (bind(rawSocket->_sockfd, (struct sockaddr*)(&rawSocket->_sockAddr),
  //            rawSocket->_socklen) == -1) {
  //     exitError("bind() failed");
  //   }
  return rawSocket;
}

int resolveDNS(const char* host, t_rawSocket* rawSocket) {
  struct addrinfo hints;
  struct addrinfo* result = NULL;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_RAW;
  hints.ai_protocol = IPPROTO_ICMP;

  int ret = getaddrinfo(host, NULL, &hints, &result);

  if (ret != 0 || result == NULL) {
    char buffer[100] = {0};
    sprintf(buffer, "ft_ping: %s: Nom ou service inconnu\n", host);
    exitProgram(buffer, 2, false);
  }

  rawSocket->_sockAddr = *(struct sockaddr_in*)result->ai_addr;
  char str_ip[16] = {0};
  inet_ntop(AF_INET, &rawSocket->_sockAddr.sin_addr.s_addr, &str_ip[0], 100);

  strncpy(&rawSocket->_ipAddress[0], &str_ip[0], 15);

  return 0;
}

void* buildIcmpHeader(void* hdrPtr) {
  struct icmphdr* header = (struct icmphdr*)hdrPtr;
  static uint16_t seqno = 0;

  header->type = ICMP_ECHO;
  header->code = 0; // network unreachable
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

  if (len == 1) { // padding
    uint8_t last = *(const uint8_t*)data;
    sum += (uint16_t)last << 8;
  }

  while (sum >> 16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  struct icmphdr* header = (struct icmphdr*)packet;
  header->checksum = (uint16_t)(~sum);
}

void sendPacket(t_ping* ping, t_rawSocket* rawSocket) {
  bzero(ping->packet, ping->packetSize);
  buildIcmpHeader(ping->packet);
  icmpChecksum(ping->packet, ping->packetSize);

  if ((size_t)sendto(rawSocket->_sockfd, ping->packet, ping->packetSize,
                     MSG_CONFIRM, (struct sockaddr*)(&rawSocket->_sockAddr),
                     rawSocket->_socklen) != ping->packetSize) {
    exitProgram("sendTo() failed", errno, true);
  }

  ++ping->stats.nbSend;
}