#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <utils.h>
#include <ft_ping.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct s_rawSocket {
  int _sockfd;
  struct sockaddr_in _sockAddr;
  socklen_t _socklen;
  char _ipAddress[16];
  char* _hostname;
} t_rawSocket;

typedef struct s_ping t_ping;

t_rawSocket* initializeRawSocket(const char* host);
int resolveDNS(const char* host, t_rawSocket* server);
void* buildIcmpHeader(void* hdrPtr);
void icmp_checksum(const void* packet, int len);
void sendPacket(t_ping* ping, t_rawSocket* rawSocket);

#endif