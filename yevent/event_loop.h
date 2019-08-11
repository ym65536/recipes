#ifndef YEVENT_EVENTLOOP_H
#define YEVENT_EVENTLOOP_H

#include <sys/epoll.h>
#include <vector>
#include <memory>
#include <mutex>
#include "thread.h"
#include "channel.h"
#include "timer_queue.h"
#include "callbacks.h"

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

  void UpdateChannel(Channel*);
  void RemoveChannel(Channel*);

  TimerId RunAt(const Timestamp& when, const TimerCallback& cb);
  TimerId RunAfter(double interval, const TimerCallback& cb);
  TimerId RunEvery(double interval, const TimerCallback& cb);

  void RunInLoop(const Functor& cb);
  void QueueInLoop(const Functor& cb);

 private:
  void DoPendingFunctors();
  void HandleRead();
  void WakeUp();

  int epollfd_;
  bool looping_;
  bool quit_;
  const pid_t tid_;
  std::unique_ptr<EPoller> epoller_;
  ChannelList channels_;
  std::unique_ptr<TimerQueue> timer_queue_;
  std::mutex mtx_;
  std::vector<Functor> pending_functors_;
  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;
};

};

#endif
