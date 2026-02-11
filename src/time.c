#include <myTime.h>

void startRTT(t_RTT* rtts) {
  if (gettimeofday(&rtts->begin, NULL) == -1) {
    exitProgram("gettimeofday() failed", errno, true);
  }
}

void endRTT(t_RTT* rtts) {
  if (gettimeofday(&rtts->end, NULL) == -1) {
    exitProgram("gettimeofday() failed", errno, true);
  }
  timersub(&rtts->end, &rtts->begin, &rtts->result);
}