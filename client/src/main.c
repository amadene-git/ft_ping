#include <ft_ping.h>
#include <utils.h>
#include <rawSocket.h>

#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

void* buildIcmpHeader(void* hdrPtr) {
  struct icmphdr* header = (struct icmphdr*)hdrPtr;
  static uint16_t seqno = 0;

  header->type = ICMP_ECHO;
  header->code = 0; // network unreachable
  // header->code = 1;// host unreachable
  // header->code = 3;// port unreachable

  // laisse a 0 pour l'instant
  header->checksum = 0;

  header->un.echo.id = getpid();
  header->un.echo.sequence = seqno++;

  return hdrPtr + sizeof(struct icmphdr);
}

uint16_t computeChecksum(char* buffer, size_t len) {

  uint16_t sum = 0;
  for (size_t i = 0; i < len; ++i) {
    sum += buffer[i];
  }

  return ~sum;
}

void insertChecksum(void* hdrPtr, uint16_t checksum) {
  struct icmphdr* header = (struct icmphdr*)hdrPtr;
  header->checksum = checksum;
}

// static uint16_t icmp_checksum(const void* buf, int len) {
//   const uint16_t* data = buf;
//   uint32_t sum = 0;

//   while (len > 1) {
//     sum += *data++;
//     len -= 2;
//   }

//   if (len == 1) {
//     uint8_t last = *(const uint8_t*)data;
//     sum += (uint16_t)last << 8; /* pad high byte (network byte order) */
//   }

//   /* fold 32-bit sum to 16 bits */
//   while (sum >> 16)
//     sum = (sum & 0xFFFF) + (sum >> 16);

//   return (uint16_t)(~sum);
// }

int main(int ac, char** av) {
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
  myLog("ft_pîng: socket initialized\n");


	pingLog("PING  (%s) [PayloadSize]([PacketSize]) bytes of data.\n", &rawSocket->_ipAddress[0]);

  size_t bufferSize = 42;
  char buffer[bufferSize];
  bzero(buffer, bufferSize);
  buildIcmpHeader(&buffer[0]);
    uint16_t checksum = computeChecksum(&buffer[0], bufferSize);
  // uint16_t checksum = icmp_checksum(&buffer[0], bufferSize);
  insertChecksum(&buffer[0], checksum);

  int ret =
      sendto(rawSocket->_sockfd, buffer, bufferSize, MSG_CONFIRM,
             (struct sockaddr*)(&rawSocket->_sockAddr), rawSocket->_socklen);
  myLog("ft_pîng: send %d bytes\n", ret);

  char recvBuffer[bufferSize];
  bzero(recvBuffer, bufferSize);
  recvfrom(rawSocket->_sockfd, recvBuffer, bufferSize, 0,
           (struct sockaddr*)(&rawSocket->_sockAddr), &rawSocket->_socklen);

  myLog("ft_pîng: recv  %s \n", recvBuffer);
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