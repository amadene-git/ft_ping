#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <netinet/in.h>

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

typedef struct s_rawSocket {
  int _sockfd;
  struct sockaddr_in _sockAddr;
  socklen_t _socklen;
  char _ipAddress[16];
} t_rawSocket;

t_rawSocket* initializeRawSocket(const char* bindingAddress);

#endif