#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Checksum volontairement faux
uint16_t bad_checksum(void* data, int len) {
  (void)data;
  (void)len;
  return (0xDEAD);  // checksum invalide
}

// Checksum correct (pour comparaison)
uint16_t good_checksum(void* data, int len) {
  uint16_t* ptr = data;
  uint32_t sum = 0;

  while (len > 1) {
    sum += *ptr++;
    len -= 2;
  }
  if (len == 1) sum += *(uint8_t*)ptr;
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  return (~sum);
}

void send_corrupted_icmp(const char* dest_ip, int corrupt) {
  int sockfd;
  struct sockaddr_in dest;
  struct icmphdr icmp;
  char packet[64];

  // Créer un raw socket
  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sockfd < 0) {
    perror("socket (besoin de root)");
    return;
  }

  // Destination
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  inet_pton(AF_INET, dest_ip, &dest.sin_addr);

  // Construire le paquet ICMP
  memset(&icmp, 0, sizeof(icmp));
  icmp.type = ICMP_ECHO;
  icmp.code = 0;
  icmp.un.echo.id = htons(1234);
  icmp.un.echo.sequence = htons(1);
  icmp.checksum = 0;

  icmp.type = ICMP_DEST_UNREACH;  // ou ICMP_TIME_EXCEEDED
  icmp.code = 0;
  icmp.checksum = good_checksum(&icmp, sizeof(icmp));  // checksum correct !

  icmp.type = ICMP_ECHOREPLY;
  icmp.code = 0;
  icmp.checksum = good_checksum(&icmp, sizeof(icmp));

  // Checksum correct ou corrompu selon le mode
//   if (corrupt)
//     icmp.checksum = bad_checksum(&icmp, sizeof(icmp));
//   else
//     icmp.checksum = good_checksum(&icmp, sizeof(icmp));

  // Copier dans le buffer
  memcpy(packet, &icmp, sizeof(icmp));

  // Envoyer
  if (sendto(sockfd, packet, sizeof(icmp), 0, (struct sockaddr*)&dest, sizeof(dest)) < 0)
    perror("sendto");
  else
    printf("Paquet ICMP %s envoyé vers %s\n", corrupt ? "CORROMPU" : "normal", dest_ip);

  close(sockfd);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <ip>\n", argv[0]);
    return (1);
  }

  printf("Envoi de 5 paquets corrompus...\n");
  for (int i = 0; i < 5; i++) send_corrupted_icmp(argv[1], 1);  // 1 = corrompu

  printf("Envoi de 5 paquets normaux...\n");
  for (int i = 0; i < 5; i++) send_corrupted_icmp(argv[1], 0);  // 0 = normal

  return (0);
}
