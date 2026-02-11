#include <ft_ping.h>
#include <list.h>
#include <netUtils.h>
#include <timeUtils.h>
#include <utils.h>

#include <signal.h>
#include <unistd.h>

#include <signal.h>
#include <stdatomic.h>

volatile sig_atomic_t g_stop = 0;

void sigHandler(int signo) {
  if (signo == SIGINT) {
    g_stop = 1;
  }
}

int main(int ac, char** av) {

  signal(SIGINT, sigHandler);

  if (ac != 2) {
    exitProgram("Usage: ./ft_ping destination", 2, false);
    return 1;
  }

  t_rawSocket* rawSocket = initializeRawSocket(av[1]);
  t_ping ping;
  ping.packetSize = 84;
  ping.seqnum = 0;
  ping.packet = galloc(ping.packetSize);
  ping.stats.nbSend = 0;
  ping.stats.nbRecv = 0;
  ping.stats.progDuration = initRTT();
  ping.stats.rtts = galloc(sizeof(t_list*));
  *(ping.stats.rtts) = NULL;

  printFirstLog(rawSocket, &ping);

  while (g_stop == 0) {
    sendPacket(&ping, rawSocket);

    char recvBuffer[ping.packetSize];
    bzero(recvBuffer, ping.packetSize);
    recvfrom(rawSocket->_sockfd, recvBuffer, ping.packetSize, 0,
             (struct sockaddr*)(&rawSocket->_sockAddr), &rawSocket->_socklen);
    computeRTT((t_RTT*)((*ping.stats.rtts)->data));

    uint8_t ttl = ((struct iphdr*)recvBuffer)->ttl;
    printLog(rawSocket, &ping, ttl);
    sleep(1);
  }
  printf("\n--- statistics ---\n");
  printf("%lu packets transmitted, ", ping.stats.nbSend);
  printf("%lu received,", ping.stats.nbSend);
  printf("%lu%% packet loss, ", computeLossPercent(ping.stats));
  printf("time %lums\n", getProgramDuration(&ping.stats.progDuration));
  printf("rtt min/avg/max/mdev = ");
  t_microsec min = getMinRtt(*ping.stats.rtts);
  printf("%lu.%lu/", min / 1000, min);
  // pritnf("%lu/X/X/X ms\n", getMinRtt(*ping.stats.rtts));
  // pritnf("%lu/X/X/X ms\n", getMinRtt(*ping.stats.rtts));
  exitProgram("", EXIT_SUCCESS, false);
}
