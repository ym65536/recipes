#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include "thread/Thread.h"
#include <sys/epoll.h>
#include <string>
#include <vector>
#include <memory>
#include "TimerQueue.h"
#include "thread/Mutex.h"

namespace muduo {

class Channel;
class EPoller;

const static uint32_t EPOLL_FD_SIZE = 102400;

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  void loop();
  void UpdateChannel(Channel* channel);

  void quit();

  void AssertInLoopThread() {
    if (!isInLoopThread()) {
      abortInLoop("Not in loop thread");
    }
  }

  bool isInLoopThread() const {
    return thread_id_ == CurrentThread::tid();
  }

  void abortInLoop(const std::string&);
  EventLoop* getCurrentLoop();

  void RunInLoop(const Functor&);
  void QueueInLoop(const Functor& cb);
  void doPendingFunctors();

  void HandleRead();
  void wakeup();

  void RemoveChannel(Channel* channel);

  TimerId RunAt(const Timestamp& when, const TimeoutCallback& cb);
  TimerId RunAfter(double interval, const TimeoutCallback& cb);
  TimerId RunEvery(double interval, const TimeoutCallback& cb);

 private:
  typedef std::vector<Channel*> ChannelList;
  typedef std::unique_ptr<EPoller> EPollerPtr;
  typedef std::unique_ptr<TimerQueue> TimerQueuePtr;

  const pid_t thread_id_ = CurrentThread::tid();
  bool looping_ = false;
  bool quit_ = false;
  bool call_pending_functors_ = false;
  ChannelList active_channels_;
  EPollerPtr epoller_;
  TimerQueuePtr timer_queue_;
  MutexLock mutex_;
  int wakeup_fd_;
  Channel wakeup_channel_;
  std::vector<Functor> pending_functors_;
};

};

#endif
