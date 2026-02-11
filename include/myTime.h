#ifndef TIME_H
#define TIME_H

#include <sys/time.h>
#include <utils.h>

typedef struct s_RTT {
  struct timeval begin;
  struct timeval end;
  struct timeval result;
} t_RTT;

void startRTT(t_RTT* rtts);
void endRTT(t_RTT* rtts);

#endif