#include <stats.h>

// std
#include <math.h>

uint64_t computeLossPercent(t_stats stats) {
  if (stats.nbSend == 0) {
    return 0;
  }
  return ((stats.nbSend - stats.nbRecv) * 100) / stats.nbSend;
}

void initializeStats(t_stats* stats) {
  stats->nbSend = 0;
  stats->nbRecv = 0;

  stats->maxRtt = 0;
  stats->minRtt = (uint64_t)-1;  // uint64 max
  stats->meanRtt = 0;
  stats->sumDeltas = 0;
}

void updateStats(t_stats* stats) {
  ++stats->nbRecv;

  t_microsec duration = timevalToUs(stats->currentRTT.result);
  if (duration < stats->minRtt) {
    stats->minRtt = duration;
  }
  if (duration > stats->maxRtt) {
    stats->maxRtt = duration;
  }

  double delta1 = (double)duration - stats->meanRtt;
  stats->meanRtt += delta1 / stats->nbRecv;

  double delta2 = (double)duration - stats->meanRtt;
  stats->sumDeltas += delta1 * delta2;
}

t_microsec getStdDev(t_stats* stats) {
  if (stats->nbRecv < 2) {
    return 0;
  }
  return (t_microsec)(sqrt(stats->sumDeltas / stats->nbRecv));
}