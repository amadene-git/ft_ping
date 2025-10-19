#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/*
struct sockaddr_in {
    sa_family_t    sin_family;
    uint16_t       sin_port;
    struct in_addr sin_addr;
};

struct in_addr {
    uint32_t       s_addr;
};
*/

typedef struct s_pingClient {
  int _sockfd;
  struct sockaddr_in _sockAddr;
  socklen_t _socklen;
  char _ipAddress[16];
} t_pingClient;

#endif