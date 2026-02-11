#ifndef STATS_H
#define STATS_H

#include <timeUtils.h>
#include <stdint.h>

// void* myAlloc(size_t size, list) {

//     void * ptr = malloc(size);
//     if (ptr)
//     pushBackList(ptr, list);
// }

typedef struct s_RTT t_RTT;


typedef struct s_stats {
  uint64_t nbSend;
  uint64_t nbRecv;
  uint64_t time;
  t_RTT* rtts;
} t_stats;

#endif