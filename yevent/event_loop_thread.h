#ifndef YEVENT_EVENT_LOOP_THREAD_H
#define YEVENT_EVENT_LOOP_THREAD_H

#include "thread.h"
#include <mutex>
#include <condition_variable>

namespace yevent {

class EventLoop;

class EventLoopThread {
 public:
  EventLoopThread();
  ~EventLoopThread();
  
  EventLoop* StartLoop();

 private:
  EventLoop* loop_;
  Thread thread_;
  std::mutex mtx_;
  std::condition_variable cv_;
};


}

#endif
