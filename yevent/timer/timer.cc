#include "timer.h"

using namespace yevent;

void Timer::Restart(const Timestamp& now) {
  if (Repeat()) {
    when_ = addTime(now, interval_);
  } else {
    when_ = Timestamp::invalid();
  }
}

