#ifndef YEVENT_EVENTLOOP_H
#define YEVENT_EVENTLOOP_H

#include "thread.h"
#include <sys/epoll.h>

const static uint32_t EPOLL_FD_SIZE = 65536;

namespace yevent {

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  void Loop();

  void AssertInLoop();
  bool IsInLoop() {
    return tid_ == CurrentThread::tid();
  }

  EventLoop* GetLoop();

 private:
  int epollfd_;
  bool looping_;
  const pid_t tid_;
};

};

#endif
