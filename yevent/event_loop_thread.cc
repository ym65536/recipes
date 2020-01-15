#include "event_loop_thread.h"
#include "event_loop.h"
#include "logging.h"

using namespace yevent;

EventLoopThread::EventLoopThread() : 
  loop_(nullptr),
  thread_([this]() {
    EventLoop loop;
    {
      std::lock_guard<std::mutex> lock(mtx_);
      loop_ = &loop;
      cv_.notify_all();
    }
    loop_->Loop(); }) {
}

EventLoopThread::~EventLoopThread() {
  loop_->Quit();
  thread_.join();
}

EventLoop* EventLoopThread::StartLoop() {
  thread_.start();
  {
    std::unique_lock<std::mutex> lock(mtx_);
    while (loop_ == nullptr) {
      cv_.wait(lock);
    }
  }
  return loop_;
}

