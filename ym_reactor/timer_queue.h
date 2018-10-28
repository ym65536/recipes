#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <datetime/Timestamp.h>
#include "channel.h"
#include "timer.h"
#include <set>
#include <vector>

namespace muduo {

class EventLoop;

typedef Timer*  TimerId;

class TimerQueue {
 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  TimerId addTimer(const TimeoutCallback& cb, const Timestamp& when, double interval);

 private:
  typedef std::shared_ptr<Timer> TimerPtr;
  typedef std::pair<Timestamp, TimerPtr> TimerEntry;
  typedef std::set<TimerEntry> TimerList;
  
  void addTimerInLoop(const TimerPtr& timer);
  void HandleRead();
  void getExpiredTimer(const Timestamp& now, std::vector<TimerEntry>&);
  
  bool insertTimer(const TimerPtr& timer);

  EventLoop* loop_;
  TimerList timers_;
  int timerfd_;
  Channel timer_channel_;
};

};

#endif

