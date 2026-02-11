#ifndef TIME_H
#define TIME_H

#include <sys/time.h>
#include <stdint.h>

typedef struct s_RTT {
  struct timeval begin;
  struct timeval end;
  struct timeval result;
} t_RTT;

typedef uint64_t t_microsec;
typedef uint64_t t_millisec;
typedef uint64_t t_sec;

t_RTT initRTT();
void computeRTT(t_RTT* rtt);
uint64_t timevalToMs(struct timeval tv);
uint64_t timevalToUs(struct timeval tv);

#endif