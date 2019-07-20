#ifndef YEVENT_TIMER_H
#define YEVENT_TIMER_H

#include "timestamp.h"
#include "callbacks.h"

namespace yevent {

class Timer {
 public:
  Timer(const TimerCallback& cb, const Timestamp& when, double interval):
    cb_(cb), when_(when), interval_(interval) {
  }
  ~Timer() {
  }

  void Run() const {
    cb_();
  }

  Timestamp When() {
    return when_;
  }

  bool Repeat() {
    return interval_ > 0.0;
  }

  void Restart(const Timestamp& now);

 private:
  TimerCallback cb_;
  Timestamp when_;
  double interval_;
};

typedef Timer* TimerId;

};

#endif
