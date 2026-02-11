#include <timeUtils.h>

t_RTT initRTT() {
  t_RTT  rtt = {0};
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

//   t_list* new = newElem(rtts);
//   pushBack(rttList, new);
}