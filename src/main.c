#include <ft_ping.h>
#include <netUtils.h>
#include <utils.h>
#include <myTime.h>

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

  t_RTT rtts;

  while (1) {
    bzero(ping.packet, ping.packetSize);
    buildIcmpHeader(ping.packet);
    icmp_checksum(ping.packet, ping.packetSize);

    sendto(rawSocket->_sockfd, ping.packet, ping.packetSize, MSG_CONFIRM,
           (struct sockaddr*)(&rawSocket->_sockAddr), rawSocket->_socklen);

    startRTT(&rtts);

    char recvBuffer[ping.packetSize];
    bzero(recvBuffer, ping.packetSize);
    recvfrom(rawSocket->_sockfd, recvBuffer, ping.packetSize, 0,
             (struct sockaddr*)(&rawSocket->_sockAddr), &rawSocket->_socklen);

    uint8_t ttl = ((struct iphdr*)recvBuffer)->ttl;
    sleep(1);

    printLog(rawSocket, &ping, ttl, rtts.result);
  }
}
