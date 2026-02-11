#include <netUtils.h>
#include <utils.h>

t_RTT initRTT() {
  t_RTT rtt = {0};
  if (gettimeofday(&rtt.begin, NULL) == -1) {
    exitProgram("gettimeofday() failed", errno, true);
  }
  return rtt;
}

void computeRTT(t_RTT* rtt) {
  if (gettimeofday(&rtt->end, NULL) == -1) {
    exitProgram("gettimeofday() failed", errno, true);
  }
  timersub(&rtt->end, &rtt->begin, &rtt->result);
}

uint64_t timevalToMs(struct timeval tv) {
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

uint64_t timevalToUs(struct timeval tv) {
  return (tv.tv_sec * 1000000) + tv.tv_usec;
}