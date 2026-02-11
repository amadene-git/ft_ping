#include <utils.h>
#include <timeUtils.h>

void exitProgram(const char* message, int code, bool hasErrno) {
  if (code != EXIT_SUCCESS) {
    dprintf(2, "%s", message);
    if (hasErrno)
      dprintf(2, " (errno: %s)", strerror(errno));
    dprintf(2, "\n");
  }
  freeGarbage();
  exit(code);
}

void printFirstLog(t_rawSocket* rawSocket, t_ping* ping) {
  static const size_t HEADERS_SIZE =
      sizeof(struct iphdr) + sizeof(struct icmphdr);

  printf("PING %s (%s) %lu(%lu) bytes of data.\n", rawSocket->_hostname,
         &rawSocket->_ipAddress[0], ping->packetSize - HEADERS_SIZE,
         ping->packetSize);
}
void printLog(t_rawSocket* rawSocket, t_ping* ping, uint8_t ttl) {
  t_microsec rtt = timevalToUs(((t_RTT*)(*ping->stats.rtts)->data)->result);
  printf("%lu bytes from %s (%s): icmp_seq=%ld ttl=%hu time=%lu.%lu ms\n",
         ping->packetSize, rawSocket->_hostname, rawSocket->_ipAddress,
         ping->seqnum++, ttl, rtt / 1000, rtt % 1000);
}

char* ft_strdup(const char* s) {
  size_t size = strlen(s) + 1;
  char* ret = galloc(size);
  while (*s) {
    *ret++ = *s++;
  }
  *ret = 0;
  return (ret - (size - 1));
}
