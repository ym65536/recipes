#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include "thread/Thread.h"
#include <sys/epoll.h>
#include <string>

namespace muduo {

const static uint32_t EPOLL_FD_SIZE = 102400;

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  void loop();
  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortInLoop("Not in loop thread");
    }
  }

  bool isInLoopThread() const {
    return thread_id_ == CurrentThread::tid();
  }

  void abortInLoop(const std::string&);
  EventLoop* getCurrentLoop();

 private:
  int epoll_fd_ = -1;
  const pid_t thread_id_;
  bool looping_ = false;
};

};

#endif
