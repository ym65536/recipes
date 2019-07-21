#ifndef YEVENT_TIMER_QUEUE_H
#define YEVENT_TIMER_QUEUE_H

#include "timestamp.h"
#include "timer.h"
#include <memory>
#include <map>

namespace yevent {

class EventLoop;
class Channel;

class TimerQueue {
 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();

  TimerId AddTimer(const TimerCallback& cb, 
                   const Timestamp& when, 
                   double interval);
  //void CancelTimer(TimerId);

 private:
  typedef std::multimap<Timestamp, Timer*> TimerList;

  void ResetTimerFd(const Timestamp& when);
  uint64_t ReadTimerFd();

  void AddTimerInLoop(Timer* timer);
  void HandleRead();
  bool InsertTimer(Timer*);

  EventLoop* loop_;
  int timer_fd_;
  std::unique_ptr<Channel> chann_;
  TimerList timers_;
};

};

#endif
