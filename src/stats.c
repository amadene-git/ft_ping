#include <stats.h>
#include <stdio.h>

uint64_t computeLossPercent(t_stats stats) {
  return ((stats.nbSend - stats.nbRecv) * 100) / stats.nbSend;
}

uint64_t getProgramDuration(t_RTT* progDuration) {
  computeRTT(progDuration);
  return timevalToMs(progDuration->result);
}

t_microsec getMinRtt(t_list *rtts) {
  t_microsec min = timevalToUs(((t_RTT*)rtts->data)->result);
  while (rtts) {
    if (min > timevalToUs(((t_RTT*)rtts->data)->result)){
      min = timevalToUs(((t_RTT*)rtts->data)->result);
    }
    rtts = rtts->next;
  }
  return min;
}
