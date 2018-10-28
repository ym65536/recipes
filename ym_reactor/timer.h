#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <datetime/Timestamp.h>
#include "callbacks.h"

namespace muduo {

class Timer {
 public:
  Timer(const TimeoutCallback& cb, const Timestamp& when, double interval) :
    interval_(interval), repeat_(interval > 0.0), cb_(cb), when_(when) {
  }
  ~Timer() {
  }

  void run() const {
    cb_();
  }

  Timestamp when() { return when_; }

  bool repeat() {return repeat_; } 

  void restart(const Timestamp& now);

 private:
  double interval_;
  bool repeat_;
  TimeoutCallback cb_;
  Timestamp when_;
};

};

#endif
