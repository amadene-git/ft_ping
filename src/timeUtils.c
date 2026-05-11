#include <netUtils.h>
#include <utils.h>

void initializeCurrentRtt(t_ping* ping) {
  if (gettimeofday(&ping->stats.currentRTT.begin, NULL) == -1) {
    exitProgram("gettimeofday() failed", errno, true, ping);
  }
}

void computeCurrentRtt(t_ping* ping) {
  if (gettimeofday(&ping->stats.currentRTT.end, NULL) == -1) {
    exitProgram("gettimeofday() failed", errno, true, ping);
  }
  timersub(&ping->stats.currentRTT.end, &ping->stats.currentRTT.begin, &ping->stats.currentRTT.result);
}

t_millisec timevalToMs(struct timeval tv) {
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

t_microsec timevalToUs(struct timeval tv) {
  return (tv.tv_sec * 1000000) + tv.tv_usec;
}
