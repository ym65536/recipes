#include "event_loop_thread_pool.h"
#include "event_loop.h"
#include "event_loop_thread.h"
#include <assert.h>

using namespace yevent;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop)
  : base_loop_(base_loop),
    started_(false),
    thread_num_(0),
    next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
  // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::Start() {
  assert(!started_);
  base_loop_->AssertInLoop();

  started_ = true;
  for (int i = 0; i < thread_num_; ++i) {
    EventLoopThread* t = new EventLoopThread;
    threads_.push_back(t);
    loops_.push_back(t->StartLoop());
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  base_loop_->AssertInLoop();
  EventLoop* loop = base_loop_;
  if (!loops_.empty()) {
    // round-robin
    loop = loops_[next_];
    next_ = (next_ + 1) % loops_.size();
  }
  return loop;
}

