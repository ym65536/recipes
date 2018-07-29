#include "EventLoopThread.h"
#include "EventLoop.h"
#include "logging/Logging.h"

using namespace muduo;

EventLoopThread::EventLoopThread() : 
  loop_(nullptr),
  thread_(std::bind(&EventLoopThread::threadFunc, this)), 
  mutex_(), cond_(mutex_) {
}

EventLoopThread::~EventLoopThread() {
  loop_->quit();
  thread_.join();
}

EventLoop* EventLoopThread::startLoop() {
  thread_.start();

  {
    MutexLockGuard lock(mutex_);
    while (loop_ == nullptr) {
      cond_.wait();
    }
  }

  return loop_;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;
  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop_->loop();
}
