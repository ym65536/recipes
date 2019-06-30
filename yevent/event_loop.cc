#include <unistd.h>
#include "event_loop.h"
#include "logging.h"

using namespace std;
using namespace yevent;

__thread EventLoop* t_this_loop = nullptr;

EventLoop::EventLoop(): epollfd_(-1), looping_(false), 
    tid_(CurrentThread::tid()) {
  epollfd_ = epoll_create(EPOLL_FD_SIZE);
  if (epollfd_ == -1) {
    LOG_SYSERR << "epoll_create1";
    exit(EXIT_FAILURE);
  }
  if (t_this_loop) {
    LOG_FATAL << "Another EventLoop=" << t_this_loop << " exist in thread=" << tid_;
  }
  t_this_loop = this;
  LOG_TRACE << "EventLoop=" << t_this_loop << " create in thread=" << tid_
      << ",epoll fd=" << epollfd_;
} 

EventLoop::~EventLoop() {
  if (epollfd_ > 0) {
    close(epollfd_);
  }
}

void EventLoop::Loop() {
  assert(looping_ == false);
  AssertInLoop();
  looping_ = true;
  
  epoll_wait(epollfd_, nullptr, EPOLL_FD_SIZE, 5 * 1000);

  LOG_TRACE << "EventLoop=" << t_this_loop << " stop loop.";
  looping_ = false;
} 

void EventLoop::AssertInLoop() {
  if (!IsInLoop()) {
    LOG_FATAL << "My loop=" << t_this_loop << ",tid=" << tid_ 
        << ",current_tid=" << CurrentThread::tid();
  }
}

EventLoop* EventLoop::GetLoop() {
  return t_this_loop;
}
