#include <ft_ping.h>
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

uint16_t computeChecksum(char *buffer, size_t len) {

	uint16_t sum = 0;
	for (size_t i = 0; i < len; ++i) {
		sum += buffer[i];
	}

	return sum;

}

void insertChecksum(void* hdrPtr, uint16_t checksum) {
  struct icmphdr* header = (struct icmphdr*)hdrPtr;
  header->checksum = checksum;
}

int main(int ac, char** av) {
  if (ac != 2) {
    dprintf(2, "Error: bad args\nUsage: ./pingServer IP\n");
    return 1;
  }

  t_rawSocket* server = NULL;
  t_rawSocket* rawSocket = initializeRawSocket(av[1], server);
  if (rawSocket == NULL) {
    printf("EXIT\n");
    return 1;
  }
  printf("ft_pîng: socket initialized\n");

  size_t bufferSize = 42;
  char buffer[bufferSize];
  bzero(buffer, bufferSize);
  buildIcmpHeader(&buffer[0]);
  uint16_t checksum = computeChecksum(&buffer[0], bufferSize);
	insertChecksum(&buffer[0], checksum);
  
	
	int ret =
      sendto(rawSocket->_sockfd, buffer, bufferSize, MSG_CONFIRM,
             (struct sockaddr*)(&rawSocket->_sockAddr), rawSocket->_socklen);
  printf("ft_pîng: send %d bytes\n", ret);

  char recvBuffer[bufferSize];
  bzero(recvBuffer, bufferSize);
  recvfrom(rawSocket->_sockfd, recvBuffer, bufferSize, 0,
           (struct sockaddr*)(&rawSocket->_sockAddr), &rawSocket->_socklen);

  printf("ft_pîng: recv  %s \n", recvBuffer);
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