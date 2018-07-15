#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include "thread/Thread.h"
#include <sys/epoll.h>
#include <string>
#include <memory>

namespace muduo {

class Channel;
class EPoller;

const static uint32_t EPOLL_FD_SIZE = 102400;

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  void loop();
  void updateChannel(Channel* channel);

  void quit();

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortInLoop("Not in loop thread");
    }
  }

  bool isInLoopThread() const {
    return thread_id_ == CurrentThread::tid();
  }

  void abortInLoop(const std::string&);
  EventLoop* getCurrentLoop();

 private:
  typedef std::vector<Channel*> ChannelList;
  typedef std::unique_ptr<EPoller> EPollerPtr;

  const pid_t thread_id_;
  bool looping_ = false;
  bool quit_ = false;
  ChannelList active_channels_;
  EPollerPtr epoller_;
};

};

#endif
