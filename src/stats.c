#include <stats.h>

// std
#include <math.h>

uint64_t computeLossPercent(t_stats stats) {
  return ((stats.nbSend - stats.nbRecv) * 100) / stats.nbSend;
}

uint64_t getProgramDuration(t_RTT* progDuration) {
  computeRTT(progDuration);
  return timevalToMs(progDuration->result);
}

t_microsec getMinRtt(t_list* rtts) {
  t_microsec min = timevalToUs(((t_RTT*)rtts->data)->result);
  while (rtts) {
    if (min > timevalToUs(((t_RTT*)rtts->data)->result)) {
      min = timevalToUs(((t_RTT*)rtts->data)->result);
    }
    rtts = rtts->next;
  }
  return min;
}

t_microsec getMaxRtt(t_list* rtts) {
  t_microsec max = timevalToUs(((t_RTT*)rtts->data)->result);
  while (rtts) {
    if (max < timevalToUs(((t_RTT*)rtts->data)->result)) {
      max = timevalToUs(((t_RTT*)rtts->data)->result);
    }
    rtts = rtts->next;
  }
  return max;
}

t_microsec getAverageRtt(t_list* rtts) {
  t_microsec avg = 0;
  t_microsec rest = 0;
  size_t len = listLen(rtts);
  if (len == 0) {
    return 0;
  }
  while (rtts) {
    t_microsec rtt = timevalToUs(((t_RTT*)rtts->data)->result);
    avg += rtt / len;
    rest += rtt % len;
    if (rest >= len) {
      avg += rest / len;
      rest -= len * (rest / len);
    }
    rtts = rtts->next;
  }
  return avg;
}

t_rtt_stats welfordAlgo(t_list* rtts) {
  t_rtt_stats stats = {0, 0};

  if (!rtts) {
    return stats;
  };

  size_t n = 0;
  long double mean = 0.0L;
  long double M2 = 0.0L;

  while (rtts) {
    t_microsec rtt = timevalToUs(((t_RTT*)rtts->data)->result);
    ++n;

    long double delta = rtt - mean;
    mean += delta / n;
    long double delta2 = rtt - mean;
    M2 += delta * delta2;

    rtts = rtts->next;
  }

  stats.avg = (t_microsec)mean;
  stats.mdev = (t_microsec)sqrt(M2 / n);

  return stats;
}
