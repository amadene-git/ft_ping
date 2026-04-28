#include <netUtils.h>
#include <timeUtils.h>
#include <utils.h>

// std
#include <errno.h>
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

void printFirstLog(t_ping* ping) {
  static const size_t ICMP_HEADER_SIZE = sizeof(struct icmphdr);
  static const size_t IP_HEADER_SIZE = sizeof(struct iphdr);

  printf("FT_PING %s (%s) %zu(%zu) bytes of data.\n",
         ping->rawSocket->_hostname,
         &ping->rawSocket->_ipAddress[0],
         ping->packetSize - ICMP_HEADER_SIZE,
         ping->packetSize + IP_HEADER_SIZE);
}
void printLog(t_ping* ping, ssize_t nbBytesRecv, uint8_t ttl) {
  t_microsec rtt = timevalToUs(((t_RTT*)(*ping->stats.rtts)->data)->result);
  if (strcmp(ping->rawSocket->_hostname, ping->rawSocket->_ipAddress)) {
    printf("%zd bytes from %s (%s): icmp_seq=%ld ttl=%hu time=%lu.%03lu ms\n",
           nbBytesRecv,
           ping->rawSocket->_hostname,
           ping->rawSocket->_ipAddress,
           ping->seqnum++,
           ttl,
           rtt / 1000,
           rtt % 1000);
  } else {
    printf("%zd bytes from %s: icmp_seq=%ld ttl=%hu time=%lu.%03lu ms\n",
           nbBytesRecv,
           ping->rawSocket->_hostname,
           ping->seqnum++,
           ttl,
           rtt / 1000,
           rtt % 1000);
  }
}

void printStats(t_ping* ping) {
  printf("\n--- %s ping statistics ---\n", ping->rawSocket->_hostname);
  printf("%lu packets transmitted, ", ping->stats.nbSend);
  printf("%lu received, ", ping->stats.nbRecv);
  printf("%lu%% packet loss, ", computeLossPercent(ping->stats));
  printf("time %lums\n", getProgramDuration(&ping->stats.progDuration, ping));
  printf("rtt min/avg/max/mdev = ");

  t_microsec min = getMinRtt(*ping->stats.rtts);
  printf("%lu.%lu/", min / 1000, min % 1000);

  t_rtt_stats meanAndDev = welfordAlgo(*ping->stats.rtts);
  printf("%lu.%lu/", meanAndDev.avg / 1000, meanAndDev.avg % 1000);

  t_microsec max = getMaxRtt(*ping->stats.rtts);
  printf("%lu.%lu/", max / 1000, max % 1000);

  printf("%lu.%lums\n", meanAndDev.mdev / 1000, meanAndDev.mdev % 1000);
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
