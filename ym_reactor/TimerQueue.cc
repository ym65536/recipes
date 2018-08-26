#include <sys/timerfd.h>
#include "TimerQueue.h"
#include "logging/Logging.h"
#include "EventLoop.h"
#include <datetime/Timestamp.h>
#include <algorithm>

using namespace std;
using namespace muduo;

int createTimerFd() {
  int ret = timerfd_create(CLOCK_MONOTONIC, 0);
  if (ret < 0) {
    LOG_SYSERR << "timerfd create fail";
  }
  return ret;
}

struct timespec timeLeft(const Timestamp& when) {
  int64_t ms = when.microSecondsSinceEpoch() -
    Timestamp::now().microSecondsSinceEpoch();
  struct timespec ts;
  ts.tv_sec = (time_t)(ms / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = (ms % Timestamp::kMicroSecondsPerSecond) * 1000;

  return ts;
}

uint64_t readTimerFd(int timer_fd) {
  uint64_t howmany;
  int nbytes = ::read(timer_fd, &howmany, sizeof(howmany));
  if (nbytes != sizeof(howmany)) {
    LOG_SYSERR << "Read timer fd size=" << nbytes;
    return -1;
  }
  LOG_DEBUG << "Read timer succ. value=" << howmany;
  return howmany;
}

void resetTimerFd(int timer_fd, const Timestamp& when) {
  itimerspec last_timer;
  bzero(&last_timer, sizeof(itimerspec));
  last_timer.it_value = timeLeft(when);
  LOG_DEBUG << "Reset timerfd " << timer_fd << " at " << 
    "ts=" << last_timer.it_value.tv_sec;
  int ret = timerfd_settime(timer_fd, 0, &last_timer, NULL);
  if (ret < 0) {
    LOG_SYSERR << "timerfd_settime fail.";
  }
}

TimerQueue::TimerQueue(EventLoop* loop) :
  loop_(loop), timerfd_(createTimerFd()),
  timer_channel_(loop_, timerfd_) {
  timer_channel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
  timer_channel_.EnableReading();
}

TimerQueue::~TimerQueue() {
}

TimerId TimerQueue::addTimer(const TimeoutCallback& cb, const Timestamp& when, 
    double interval) {
  TimerPtr timer = make_shared<Timer>(cb, when, interval);
  loop_->RunInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
  return timer.get();
}

void TimerQueue::addTimerInLoop(const TimerPtr& timer) {
  loop_->AssertInLoopThread();
  bool early = insertTimer(timer);
  if (early) {
    resetTimerFd(timerfd_, timer->when());
  }
}

bool TimerQueue::insertTimer(const TimerPtr& timer) {
  bool early = false;
  const Timestamp when = timer->when();
  auto it = timers_.begin();
  if (it != timers_.end()) {
    if (when < it->first) {
      early = true;
    }
  } else {
    early = true;
  }
  timers_.insert(make_pair(when, timer));
  return early;
}

void TimerQueue::getExpiredTimer(const Timestamp& now, 
    vector<TimerEntry>& expired_timer) {
  TimerEntry entry = 
    std::make_pair(addTime(now, 0.000001), nullptr);
  TimerList::const_iterator it = timers_.lower_bound(entry);
  assert(it == timers_.end() || now < it->first);
  std::copy(timers_.begin(), it, back_inserter(expired_timer));
  timers_.erase(timers_.begin(), it);
}

void TimerQueue::HandleRead() {
  loop_->AssertInLoopThread();

  Timestamp now(Timestamp::now());
  readTimerFd(timerfd_);

  std::vector<TimerEntry> expired_timer;
  getExpiredTimer(now, expired_timer);

  for (auto& entry : expired_timer) {
    entry.second->run();
  }

  for (auto& entry : expired_timer) {
    if (entry.second->repeat()) {
      entry.second->restart(now);
      insertTimer(entry.second);
    } else {
      // release this timer
    }
  }

  Timestamp early;
  if (!timers_.empty()) {
    Timestamp early = timers_.begin()->second->when();
    resetTimerFd(timerfd_, early);
  }
}
