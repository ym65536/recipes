#include "Timer.h"

using namespace muduo;

void Timer::restart(const Timestamp& now) {
  when_ = repeat_ ? addTime(now, interval_): Timestamp::invalid();
}

