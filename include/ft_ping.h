#ifndef FT_PING_H
#define FT_PING_H

#include <netUtils.h>
#include <stats.h>

// std
#include <stdint.h>
#include <string.h>

typedef struct s_rawSocket t_rawSocket;
typedef struct s_cmdLineParser t_cmdLineParser;

typedef struct s_ping {
  size_t packetSize;
  char* packet;
  uint64_t seqnum;
  t_stats stats;
  int sockfd;
  t_rawSocket* rawSocket;
  t_cmdLineParser* cmdLineParser;
  t_list** garbage;
} t_ping;

#endif