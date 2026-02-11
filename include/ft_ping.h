#ifndef FT_PING_H
#define FT_PING_H

#include <timeUtils.h>
#include <stats.h>
#include <stdint.h>
#include <string.h>

typedef struct s_stats t_stats;

typedef struct s_ping {
  size_t packetSize;
  char* packet;
  uint64_t seqnum;
  t_stats stats;
} t_ping;

#endif