#include <utils.h>

void exitProgram(const char* message, int code, bool hasErrno) {
  if (code == EXIT_SUCCESS) {
    printf("--- INSERTY statistics ---\n");
    printf("X packets transmitted, X received, X%% packet loss, time Xms\n");
    printf("rtt min/avg/max/mdev = X/X/X/X ms\n");
  } else {
    dprintf(2, "%s", message);
    if (hasErrno)
      dprintf(2, " (errno: %s)", strerror(errno));
    dprintf(2, "\n");
  }
  exit(code);
}

void printFirstLog(t_rawSocket* rawSocket, t_ping* ping) {
  static const size_t HEADERS_SIZE =
      sizeof(struct iphdr) + sizeof(struct icmphdr);

  printf("PING %s (%s) %lu(%lu) bytes of data.\n", rawSocket->_hostname,
         &rawSocket->_ipAddress[0], ping->packetSize - HEADERS_SIZE,
         ping->packetSize);
}
void printLog(t_rawSocket* rawSocket,
              t_ping* ping,
              uint8_t ttl,
              struct timeval rtt) {
  printf("%lu bytes from %s (%s): icmp_seq=%ld ttl=%hu time=%lu.%lu ms\n",
         ping->packetSize, rawSocket->_hostname, rawSocket->_ipAddress,
         ping->seqnum++, ttl, rtt.tv_usec / 1000, rtt.tv_usec % 1000);
}
