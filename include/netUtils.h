#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <ft_ping.h>

// std
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
  struct sockaddr_in _sockAddr;
  socklen_t _socklen;
  char _ipAddress[16];
  char* _hostname;
} t_rawSocket;

typedef struct s_ping t_ping;

void initializeRawSocket(const char* hostname, t_ping* ping);
int resolveDNS(t_ping* ping);
void* buildIcmpHeader(void* hdrPtr, uint16_t seqnum);
void icmpChecksum(const void* packet, int len);
void sendPacket(t_ping* ping);
ssize_t receivePacket(t_ping* ping, uint8_t* ttl);

#endif
