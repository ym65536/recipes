#include <unistd.h>
#include "event_loop.h"
#include "logging.h"
#include "channel.h"
#include "epoller.h"

using namespace std;
using namespace yevent;

__thread EventLoop* t_this_loop = nullptr;

EventLoop::EventLoop(): epollfd_(-1), looping_(false), quit_(false), 
    tid_(CurrentThread::tid()), epoller_(new EPoller(this)) {
  if (t_this_loop) {
    LOG_FATAL << "Another EventLoop=" << t_this_loop << " exist in thread=" << tid_;
  }
  t_this_loop = this;
  LOG_TRACE << "EventLoop=" << t_this_loop << " create in thread=" << tid_
      << ",epoll fd=" << epollfd_;
} 

EventLoop::~EventLoop() {
}

void EventLoop::Loop() {
  assert(looping_ == false);
  AssertInLoop();
  looping_ = true;
 
  while (!quit_) {
    epoller_->Poll(&channels_, kEPollTimeoutMs);
    for (auto channel : channels_) {
      channel->HandleEvent();
    }
  }

  LOG_TRACE << "EventLoop=" << t_this_loop << " stop loop.";
  looping_ = false;
} 

void EventLoop::Quit() {
  quit_ = true;
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

void EventLoop::UpdateChannel(Channel* channel) {
  assert(channel);
  epoller_->UpdateChannel(channel);
}
