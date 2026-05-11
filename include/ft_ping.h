#ifndef FT_PING_H
#define FT_PING_H

#include <list.h>
#include <netUtils.h>
#include <stats.h>

// std
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct s_rawSocket t_rawSocket;

typedef struct s_ping {
  size_t packetSize;
  char* packet;
  bool verbose;
  uint64_t seqnum;
  t_stats stats;
  int sockfd;
  t_rawSocket* rawSocket;
  t_list** garbage;
} t_ping;

#endif