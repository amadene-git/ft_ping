#include <ft_ping.h>
#include <rawSocket.h>
#include <utils.h>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>

//////////////////////////////////////////////////////////
///////////////////// TIME ///////////////////////////////
typedef struct RTTs {
  struct timeval begin;
  struct timeval end;
  struct timeval result;
} RTTs;

void startRTT(RTTs* rtts) {
  if (gettimeofday(&rtts->begin, NULL) == -1) {
    exitError("gettimeofday() failed");
  }
}

void endRTT(RTTs* rtts) {
  if (gettimeofday(&rtts->end, NULL) == -1) {
    exitError("gettimeofday() failed");
  }
  timersub(&rtts->end, &rtts->begin, &rtts->result);
}
///////////////////// TIME ///////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
///////////////////// BUILD PACKET ///////////////////////
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

void icmp_checksum(const void* packet, int len) {
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
///////////////////// BUILD PACKET ///////////////////////
//////////////////////////////////////////////////////////

void sigHandler(int signo) {
  if (signo == 2) {
    exit(1);
  }
}

int main(int ac, char** av) {

  signal(SIGINT, sigHandler);

  if (ac != 2) {
    myLog("Error: bad args\nUsage: ./pingServer IP\n");
    return 1;
  }

  t_rawSocket* server = NULL;
  t_rawSocket* rawSocket = initializeRawSocket(av[1], server);
  if (rawSocket == NULL) {
    myLog("EXIT\n");
    return 1;
  }

  size_t packetSize = 84;
  size_t headersSize = sizeof(struct iphdr) + sizeof(struct icmphdr);
  char packet[packetSize];

  pingLog("PING %s (%s) %lu(%lu) bytes of data.\n", rawSocket->_hostname,
          &rawSocket->_ipAddress[0], packetSize - headersSize, packetSize);

  uint64_t seqnum = 0;
  RTTs rtts;

  while (1) {
    bzero(packet, packetSize);
    buildIcmpHeader(&packet[0]);
    icmp_checksum(&packet[0], packetSize);

    sendto(rawSocket->_sockfd, packet, packetSize, MSG_CONFIRM,
           (struct sockaddr*)(&rawSocket->_sockAddr), rawSocket->_socklen);

    startRTT(&rtts);

    char recvBuffer[packetSize];
    bzero(recvBuffer, packetSize);
    recvfrom(rawSocket->_sockfd, recvBuffer, packetSize, 0,
             (struct sockaddr*)(&rawSocket->_sockAddr), &rawSocket->_socklen);

    uint8_t ttl = ((struct iphdr*)recvBuffer)->ttl;
    sleep(1);
    pingLog("%lu bytes from %s (%s): icmp_seq=%ld ttl=%hu time=%lu.%lu ms\n",
            packetSize, rawSocket->_hostname, rawSocket->_ipAddress, seqnum++,
            ttl, rtts.result.tv_usec / 1000, rtts.result.tv_usec % 1000);
  }

  // --- google.com ping statistics ---
  // 1 packets transmitted, 1 received, 0% packet loss, time 0ms
  // rtt min/avg/max/mdev = 1.030/1.030/1.030/0.000 ms
}

// unsigned short
// calcul_du_checksum(bool liberation, unsigned short* data, int taille) {
//   unsigned long checksum = 0;
//   // ********************************************************
//   // Complément à 1 de la somme des complément à 1 sur 16 bits
//   // ********************************************************
//   while (taille > 1) {
//     if (liberation == TRUE)
//       liberation_du_jeton(); // Rend la main à la fenêtre principale
//     checksum = checksum + *data++;
//     taille = taille - sizeof(unsigned short);
//   }

//   if (taille)
//     checksum = checksum + *(unsigned char*)data;

//   checksum = (checksum >> 16) + (checksum & 0xffff);
//   checksum = checksum + (checksum >> 16);

//   return (unsigned short)(~checksum);
// }