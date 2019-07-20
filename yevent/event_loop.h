#ifndef YEVENT_EVENTLOOP_H
#define YEVENT_EVENTLOOP_H

#include <sys/epoll.h>
#include <vector>
#include <memory>
#include "thread.h"
#include "channel.h"
#include "timer_queue.h"

const static uint32_t kEPollTimeoutMs = 3000;

namespace yevent {

class Channel;
class EPoller;

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  void Loop();

  void AssertInLoop();
  bool IsInLoop() {
    return tid_ == CurrentThread::tid();
  }

  void Quit();
  EventLoop* GetLoop();

  void UpdateChannel(Channel* );

  TimerId RunAt(const Timestamp& when, const TimerCallback& cb);
  TimerId RunAfter(double interval, const TimerCallback& cb);
  TimerId RunEvery(double interval, const TimerCallback& cb);

 private:
  int epollfd_;
  bool looping_;
  bool quit_;
  const pid_t tid_;
  std::unique_ptr<EPoller> epoller_;
  ChannelList channels_;
  std::unique_ptr<TimerQueue> timer_queue_;
};

};

#endif
