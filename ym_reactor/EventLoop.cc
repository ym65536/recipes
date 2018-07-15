#include "EventLoop.h"
#include "EPoller.h"
#include <Channel.h>
#include "logging/Logging.h"
#include <assert.h>

using namespace muduo;

__thread EventLoop* t_current_loop = nullptr;
const static int EPOLL_TIMEOUT_MS = 2 * 1000;

EventLoop::EventLoop() :
  thread_id_(CurrentThread::tid()) {
  t_current_loop = this;
  epoller_.reset(new EPoller(this));
}

EventLoop::~EventLoop() {
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
  
  while (!quit_) {
    active_channels_.clear();
    LOG_DEBUG << "Go to poll at time " << Timestamp::now().toString();
    epoller_->poll(EPOLL_TIMEOUT_MS, active_channels_);
    for (auto channel : active_channels_) {
      channel->handleEvent();
    }
  }

  LOG_DEBUG << "EventLoop=" << t_current_loop << " stop loop.";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = false;
  //wakeup();
}

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  epoller_->updateChannel(channel);
}

