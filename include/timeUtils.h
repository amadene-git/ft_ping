#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <stdint.h>
#include <sys/time.h>

typedef struct s_RTT {
  struct timeval begin;
  struct timeval end;
  struct timeval result;
} t_RTT;

typedef uint64_t t_microsec;
typedef uint64_t t_millisec;
typedef uint64_t t_sec;

typedef struct s_ping t_ping;

t_RTT initRTT(t_ping* ping);
void computeRTT(t_RTT* rtt, t_ping* ping);
uint64_t timevalToMs(struct timeval tv);
uint64_t timevalToUs(struct timeval tv);

#endif