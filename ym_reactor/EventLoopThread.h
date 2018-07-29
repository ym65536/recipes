#ifndef MUDUO_NET_EVENTLOOPTHRED_H
#define MUDUO_NET_EVENTLOOPTHRED_H

#include "thread/Thread.h"
#include "thread/Mutex.h"
#include "thread/Condition.h"

namespace muduo {

class EventLoop;

class EventLoopThread {
 public:
  EventLoopThread();
  ~EventLoopThread();

  EventLoop* startLoop();

 private:
  void threadFunc();
  EventLoop* loop_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
};

}


#endif
