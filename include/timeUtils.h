#ifndef TIME_H
#define TIME_H

#include <sys/time.h>
#include <utils.h>

typedef struct s_RTT {
  struct timeval begin;
  struct timeval end;
  struct timeval result;
} t_RTT;

t_RTT initRTT();
void computeRTT(t_RTT* rtt);

#endif