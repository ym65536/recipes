#include "EventLoop.h"
#include "EPoller.h"
#include <Channel.h>
#include <TimerQueue.h>
#include "logging/Logging.h"
#include <assert.h>
#include <sys/eventfd.h>

using namespace muduo;
using namespace std;

__thread EventLoop* t_current_loop = nullptr;
const static int EPOLL_TIMEOUT_MS = 5 * 1000;

static int createEfd(void) {
  int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (event_fd < 0) {
    LOG_SYSERR << "create eventfd fail.";
    abort();
  }
  return event_fd;
}

EventLoop::EventLoop() : mutex_(), wakeup_fd_(createEfd()),
    wakeup_channel_(this, wakeup_fd_) { 
  t_current_loop = this;
  epoller_.reset(new EPoller(this));
  timer_queue_.reset(new TimerQueue(this));
  wakeup_channel_.setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeup_channel_.enableReading();
}

EventLoop::~EventLoop() {
  assert(!looping_);
  t_current_loop = nullptr;
}

void EventLoop::handleRead() {
  uint64_t counter = 0;
  int nbytes = read(wakeup_fd_, &counter, sizeof(counter));
  if (nbytes != sizeof (counter)) {
    LOG_ERROR << "wakeup read size=" << nbytes << ", expect " <<
      sizeof(counter);
  }
  LOG_DEBUG << "read eventfd=" << wakeup_fd_ << ", counter=" << counter;
}

void EventLoop::wakeup(void) {
  uint64_t counter = 1;
  int nbytes = write(wakeup_fd_, &counter, sizeof(counter));
  if (nbytes != sizeof (counter)) {
    LOG_ERROR << "wakeup write size=" << nbytes << ", expect " <<
      sizeof(counter);
  }
  LOG_DEBUG << "write eventfd=" << wakeup_fd_ << ", counter=" << counter;
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
    doPendingFunctors();
  }

  LOG_DEBUG << "EventLoop=" << t_current_loop << " stop loop.";
  looping_ = false;
}

void EventLoop::quit(void) {
  quit_ = true;
  LOG_INFO << "reset quit = " << quit_;
  if (isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(const Functor& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    {
      MutexLockGuard lock(mutex_);
      pending_functors_.push_back(cb);
    }
    wakeup();
  }
}

void EventLoop::doPendingFunctors() {
  vector<Functor> tmp_functors;
  {
    MutexLockGuard lock(mutex_);
    pending_functors_.swap(tmp_functors);
  }

  for (auto functor : tmp_functors) {
    functor();
  }
}

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  epoller_->updateChannel(channel);
}

TimerId EventLoop::runAt(const Timestamp& when, const TimeoutCallback& cb) {
  return timer_queue_->addTimer(cb, when, 0.0);
}

TimerId EventLoop::runAfter(double interval, const TimeoutCallback& cb) {
  Timestamp when = addTime(Timestamp::now(), interval);
  LOG_DEBUG << "when = " << when.toString();
  return runAt(when, cb);
}

TimerId EventLoop::runEvery(double interval, const TimeoutCallback& cb) {
  Timestamp when = addTime(Timestamp::now(), interval);
  return timer_queue_->addTimer(cb, when, interval);
}

