#include <ft_ping.h>
#include <timeUtils.h>
#include <netUtils.h>
#include <utils.h>

#include <signal.h>
#include <unistd.h>

void sigHandler(int signo) {
  if (signo == 2) {
    exitProgram("", EXIT_SUCCESS, false);
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
  ping.packet = malloc(ping.packetSize);

  printFirstLog(rawSocket, &ping);

  t_RTT rtt;

  while (1) {

    sendPacket(&ping, rawSocket);
    rtt = initRTT();

    char recvBuffer[ping.packetSize];
    bzero(recvBuffer, ping.packetSize);
    recvfrom(rawSocket->_sockfd, recvBuffer, ping.packetSize, 0,
             (struct sockaddr*)(&rawSocket->_sockAddr), &rawSocket->_socklen);

    computeRTT(&rtt);


    uint8_t ttl = ((struct iphdr*)recvBuffer)->ttl;
    printLog(rawSocket, &ping, ttl, rtt.result);
    sleep(1);
  }
}
