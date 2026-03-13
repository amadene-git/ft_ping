#include <ft_ping.h>
#include <list.h>
#include <netUtils.h>
#include <signal.h>
#include <stdatomic.h>
#include <timeUtils.h>
#include <unistd.h>
#include <utils.h>

volatile sig_atomic_t g_stop = 0;

void sigHandler(int signo) {
  if (signo == SIGINT) {
    g_stop = 1;
  }
}

int main(int ac, char** av) {
  struct sigaction sa;
  sa.sa_handler = sigHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  if (ac != 2) {
    exitProgram("Usage: ./ft_ping destination", 2, false);
    return 1;
  }

  t_rawSocket* rawSocket = initializeRawSocket(av[1]);
  t_ping ping;
  ping.packetSize = 84;
  ping.seqnum = 1;
  ping.packet = galloc(ping.packetSize);
  ping.stats.nbSend = 0;
  ping.stats.nbRecv = 0;
  ping.stats.progDuration = initRTT();
  ping.stats.rtts = galloc(sizeof(t_list*));
  ping.rawSocket = rawSocket;
  *(ping.stats.rtts) = NULL;
  uint32_t timeToSleep = 1;

  printFirstLog(&ping);

  while (g_stop == 0) {
    sendPacket(&ping);

    uint8_t ttl;
    ssize_t nbBytesRecv = receivePacket(&ping, &ttl);
    if (nbBytesRecv == -1) {
      if (g_stop == 0) {
        exitProgram("recvFrom() failed: ", errno, true);
      } else {
        continue;
      }
    }
    printLog(&ping, nbBytesRecv, ttl);
    sleep(timeToSleep);
  }
  printStats(&ping);
  exitProgram("", EXIT_SUCCESS, false);
}
