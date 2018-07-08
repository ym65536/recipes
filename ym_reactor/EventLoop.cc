#include "EventLoop.h"
#include "logging/Logging.h"
#include <assert.h>

using namespace muduo;

__thread EventLoop* t_current_loop = nullptr;

EventLoop::EventLoop() :
  thread_id_(CurrentThread::tid()) {
  epoll_fd_ = epoll_create(EPOLL_FD_SIZE);
  if (epoll_fd_ < 0) {
    LOG_FATAL << "create epoll fail.";
  }
  t_current_loop = this;
}

EventLoop::~EventLoop() {
  ::close(epoll_fd_);
  assert(!looping_);
  t_current_loop = nullptr;
}

EventLoop* EventLoop::getCurrentLoop() {
  return t_current_loop;
}

void EventLoop::abortInLoop(const std::string& msg) {
  LOG_FATAL << "error_msg=" << msg << ", loop=" << this;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();

  looping_ = true;
  
  epoll_wait(epoll_fd_, nullptr, EPOLL_FD_SIZE, 5 * 1000);

  LOG_DEBUG << "EventLoop=" << t_current_loop << " stop loop.";
  looping_ = false;
}

