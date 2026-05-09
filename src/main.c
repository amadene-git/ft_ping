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
  t_ping ping;
  ping.garbage = NULL;
  ping.sockfd = -1;

  const char* hostname = parseCommandLine(ac, av, &ping);
  initializeSignal();

  initializeRawSocket(hostname, &ping);
  ping.packetSize = sizeof(struct icmphdr) + FT_PING_PAYLOAD_SIZE;
  ping.seqnum = 0;
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
