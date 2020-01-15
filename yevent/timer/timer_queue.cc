#include "timer_queue.h"
#include "channel.h"
#include "event_loop.h"
#include "logging.h"
#include <assert.h>
#include <unistd.h>
#include <sys/timerfd.h>

using namespace yevent;

struct timespec timeLeft(const Timestamp& when) {
  int64_t ms = when.microSecondsSinceEpoch() -
    Timestamp::now().microSecondsSinceEpoch();
  struct timespec ts;
  ts.tv_sec = (time_t)(ms / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = (ms % Timestamp::kMicroSecondsPerSecond) * 1000;
  return ts;
}

TimerQueue::TimerQueue(EventLoop* loop) : loop_(loop) {
  timer_fd_ = timerfd_create(CLOCK_MONOTONIC, 0);
  assert(timer_fd_ > 0);
  chann_.reset(new Channel(loop_, timer_fd_));
  chann_->SetReadCallback([this](){
    LOG_DEBUG << "timer_fd=" << timer_fd_ << " handle read.";
    loop_->AssertInLoop();
    ReadTimerFd();

    // 获取超时time
    Timestamp now(Timestamp::now());
    std::vector<Timer*> expired_timers;
    auto it = timers_.upper_bound(now);
    for (auto item = timers_.begin(); item != it; ++ item) {
      expired_timers.push_back(item->second);
    }
    timers_.erase(timers_.begin(), it);
    
    LOG_DEBUG << "expired timers size=" << expired_timers.size();
    for (const auto timer : expired_timers) {
      timer->Run();
    }

    for (auto timer : expired_timers) {
      if (timer->Repeat()) {
        timer->Restart(now);
        InsertTimer(timer);
      } else {
        delete timer;
      }
    }

    it = timers_.begin();
    if (it != timers_.end()) {
      ResetTimerFd(it->first);
    }
  });
  chann_->EnableReading();
}

TimerQueue::~TimerQueue() {
}

uint64_t TimerQueue::ReadTimerFd() {
  uint64_t nto;
  size_t nbytes = ::read(timer_fd_, &nto, sizeof(nto));
  if (nbytes != sizeof(nto)) {
    LOG_SYSERR << "timerfd=" << timer_fd_ << ",read bytes=" << nbytes;
    return UINT64_MAX;
  }
  LOG_DEBUG << "timerfd=" << timer_fd_ << ",Read timer succ.value=" << nto;
  return nto;
}

void TimerQueue::ResetTimerFd(const Timestamp& when) {
  itimerspec last_timer;
  memset(&last_timer, 0, sizeof (itimerspec));
  last_timer.it_value = timeLeft(when);
  int ret = timerfd_settime(timer_fd_, 0, &last_timer, NULL);
  if (ret < 0) {
    LOG_SYSERR << "timerfd=" << timer_fd_ << " timerfd_settime fail.";
  }
  LOG_DEBUG << "Reset timer.fd=" << timer_fd_ << ",when=" << when.toString(); 
}

TimerId TimerQueue::AddTimer(const TimerCallback& cb, 
                             const Timestamp& when, 
                             double interval) {
  auto timer = new Timer(cb, when, interval);
  loop_->RunInLoop([this, timer]() {
      loop_->AssertInLoop();
      bool early = InsertTimer(timer);
      LOG_TRACE << "timer=" << timer << ",early=" << early;
      if (early) {
        ResetTimerFd(timer->When());
      }
    });
  LOG_DEBUG << "add timer.fd=" << timer_fd_ << ",when=" << when.toString();
  return timer;
}

bool TimerQueue::InsertTimer(Timer* timer) {
  bool early = false;
  const auto when = timer->When();
  const auto& it = timers_.begin();
  if (it == timers_.end()) {
    early = true;
  } else {
    if (when < it->first) {
      early = true;
    }
  }
  timers_.insert(std::make_pair(when, timer));
  return early;
}

