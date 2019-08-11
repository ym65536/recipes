#include <unistd.h>
#include <sys/eventfd.h>
#include "event_loop.h"
#include "logging.h"
#include "channel.h"
#include "epoller.h"

using namespace std;
using namespace yevent;

__thread EventLoop* t_this_loop = nullptr;

static int createEfd(void) {
  int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (event_fd < 0) {
    LOG_SYSERR << "create eventfd fail.";
    abort();
  }
  return event_fd;
}

EventLoop::EventLoop(): epollfd_(-1), looping_(false), quit_(false), 
    tid_(CurrentThread::tid()), epoller_(new EPoller(this)),
    timer_queue_(new TimerQueue(this)), wakeup_fd_(createEfd()),
    wakeup_channel_(new Channel(this, wakeup_fd_)) {
  if (t_this_loop) {
    LOG_FATAL << "Another EventLoop=" << t_this_loop << " exist in thread=" << tid_;
  }
  t_this_loop = this;
  LOG_TRACE << "EventLoop=" << t_this_loop << " create in thread=" << tid_
      << ",epoll fd=" << epollfd_;
  wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
  wakeup_channel_->EnableReading();
} 

EventLoop::~EventLoop() {
}

void EventLoop::Loop() {
  assert(looping_ == false);
  AssertInLoop();
  looping_ = true;
 
  while (!quit_) {
    channels_.clear();
    epoller_->Poll(&channels_, kEPollTimeoutMs);
    for (auto channel : channels_) {
      channel->HandleEvent();
    }
    DoPendingFunctors();
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

TimerId EventLoop::RunAt(const Timestamp& time, const TimerCallback& cb) {
  return timer_queue_->AddTimer(cb, time, 0.0);
}

TimerId EventLoop::RunAfter(double delay, const TimerCallback& cb) {
  Timestamp time(addTime(Timestamp::now(), delay));
  return timer_queue_->AddTimer(cb, time, 0.0);
}

TimerId EventLoop::RunEvery(double interval, const TimerCallback& cb) {
  Timestamp time(addTime(Timestamp::now(), interval));
  return timer_queue_->AddTimer(cb, time, interval);
}

void EventLoop::RunInLoop(const Functor& cb) {
  if (IsInLoop()) {
    cb();
  } else {
    QueueInLoop(cb);
  }
}

void EventLoop::QueueInLoop(const Functor& cb) {
  {
    std::lock_guard<std::mutex> lock(mtx_);
    pending_functors_.push_back(cb);
  }
  WakeUp();
}

void EventLoop::DoPendingFunctors() {
  std::vector<Functor> tmp_functors;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    tmp_functors.swap(pending_functors_);
  }
  for (auto func : tmp_functors) {
    func();
  }
}

void EventLoop::HandleRead() {
  uint64_t counter = 0;
  int nbytes = read(wakeup_fd_, &counter, sizeof(counter));
  if (nbytes != sizeof (counter)) {
    LOG_ERROR << "wakeup read size=" << nbytes << ", expect " <<
      sizeof(counter);
  }
  LOG_DEBUG << "read eventfd=" << wakeup_fd_ << ", counter=" << counter;
}

void EventLoop::WakeUp(void) {
  uint64_t counter = 1;
  int nbytes = write(wakeup_fd_, &counter, sizeof(counter));
  if (nbytes != sizeof (counter)) {
    LOG_ERROR << "wakeup write size=" << nbytes << ", expect " <<
      sizeof(counter);
  }
  LOG_DEBUG << "write eventfd=" << wakeup_fd_ << ", counter=" << counter;
}

void EventLoop::RemoveChannel(Channel* channel) {
  AssertInLoop();
  assert(channel->GetLoop() == this);
  channel->DisableAll();
  epoller_->UpdateChannel(channel);
}

