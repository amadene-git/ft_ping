#ifndef STATS_H
#define STATS_H

#include <list.h>
#include <timeUtils.h>

#include <stdint.h>

typedef struct s_RTT t_RTT;

typedef struct s_stats {
  uint64_t nbSend;
  uint64_t nbRecv;
  t_RTT progDuration;
  t_list** rtts;
} t_stats;

uint64_t computeLossPercent(t_stats stats);
uint64_t getProgramDuration(t_RTT* progDuration);
t_microsec getMinRtt(t_list* rtts);

#endif