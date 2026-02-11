#ifndef FT_PING_H
#define FT_PING_H

#include <string.h>
#include <stdint.h>

typedef struct s_ping {
  size_t packetSize;
  char* packet;
  uint64_t seqnum;

} t_ping;

#endif