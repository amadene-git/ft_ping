#include <cmdLineParser.h>
#include <ft_ping.h>
#include <list.h>
#include <netUtils.h>
#include <timeUtils.h>
#include <utils.h>

// std
#include <signal.h>
#include <stdatomic.h>
#include <unistd.h>

volatile sig_atomic_t g_stop = 0;

void sigHandler(int signo) {
  if (signo == SIGINT) {
    g_stop = 1;
  }
}

void initializeSignal() {
  struct sigaction sa;
  sa.sa_handler = sigHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
}

int main(const int ac, const char** av) {
  initializeSignal();

  t_ping ping;
  ping.garbage = NULL;
  ping.sockfd = -1;

  initializeCmdLineParser("ft_ping", "Small project for 42 School that recreates the ping command.", ac, av, &ping);
  addOptionArg(createOption('c', "count", NULL, NULL, ULONG, false, "stop after sending NUMBER packets"), &ping);
  addOptionArg(
      createOption('i', "interval", NULL, NULL, ULONG, false, "wait NUMBER seconds between sending each packet"),
      &ping);
  if (ac != 2) {
    char outBuffer[1500] = {0};
    getStrHelp(outBuffer, ping.cmdLineParser);
    exitProgram(outBuffer, 2, false, &ping);
    return 1;
  }

  initializeRawSocket(av[1], &ping);
  ping.packetSize = sizeof(struct icmphdr) + FT_PING_PAYLOAD_SIZE;
  ping.seqnum = 1;
  ping.packet = galloc(ping.packetSize, &ping);
  ping.stats.nbSend = 0;
  ping.stats.nbRecv = 0;
  ping.stats.progDuration = initRTT(&ping);
  ping.stats.rtts = galloc(sizeof(t_list*), &ping);
  *(ping.stats.rtts) = NULL;
  uint32_t timeToSleep = 1;

  printFirstLog(&ping);

  while (g_stop == 0) {
    sendPacket(&ping);

    uint8_t ttl;
    ssize_t nbBytesRecv = receivePacket(&ping, &ttl);
    if (nbBytesRecv == -1) {
      if (g_stop == 0) {
        exitProgram("recvFrom() failed: ", errno, true, &ping);
      } else {
        continue;
      }
    }
    if (nbBytesRecv == 0) {
      if (*ping.stats.rtts) {
        *ping.stats.rtts = (*ping.stats.rtts)->next;
      }
      ++ping.seqnum;
      continue;
    }
    printLog(&ping, nbBytesRecv, ttl);
    sleep(timeToSleep);
  }
  printStats(&ping);
  exitProgram("", EXIT_SUCCESS, false, &ping);
}
