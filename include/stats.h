#ifndef STATS_H
#define STATS_H

#include <list.h>
#include <timeUtils.h>

// std
#include <stdint.h>

typedef struct s_RTT t_RTT;

typedef struct {
  t_microsec avg;
  t_microsec mdev;
} t_rtt_stats;
typedef struct s_stats {
  uint64_t nbSend;
  uint64_t nbRecv;
  t_RTT progDuration;
  t_list** rtts;
} t_stats;

uint64_t computeLossPercent(t_stats stats);
uint64_t getProgramDuration(t_RTT* progDuration);
t_microsec getMinRtt(t_list* rtts);
t_microsec getMaxRtt(t_list* rtts);
t_microsec getAverageRtt(t_list* rtts);
t_rtt_stats welfordAlgo(t_list* rtts);

#endif