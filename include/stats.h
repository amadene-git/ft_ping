#ifndef STATS_H
#define STATS_H

#include <timeUtils.h>

// std
#include <stdint.h>

typedef struct s_stats {
  uint64_t nbSend;
  uint64_t nbRecv;
  t_microsec minRtt;
  t_microsec maxRtt;
  t_microsec meanRtt;
  double sumDeltas;
  t_rtt currentRTT;
} t_stats;

uint64_t computeLossPercent(t_stats stats);
void initializeStats(t_stats* stats);
void updateStats(t_stats* stats);
t_microsec getStdDev(t_stats* stats);

#endif