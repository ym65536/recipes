#ifndef YEVENT_NET_EVENTLOOPTHREADPOOL_H
#define YEVENT_NET_EVENTLOOPTHREADPOOL_H

#include <condition_variable>
#include <mutex>
#include "thread.h"
#include <vector>

namespace yevent {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  EventLoopThreadPool(EventLoop* base_loop);
  ~EventLoopThreadPool();
  void SetThreadNum(int thread_num) { thread_num_ = thread_num; }
  void Start();
  EventLoop* GetNextLoop();

 private:
  EventLoop* base_loop_;
  bool started_;
  int thread_num_;
  int next_;  // always in loop thread
  std::vector<EventLoopThread*> threads_;
  std::vector<EventLoop*> loops_;
};

}

#endif 

