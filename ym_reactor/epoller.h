#ifndef MUDUO_NET_EPOLLER_H
#define MUDUO_NET_EPOLLER_H

#include "thread/Thread.h"
#include "datetime/Timestamp.h"
#include <sys/epoll.h>
#include <string>
#include <map>
#include <vector>
#include "event_loop.h"

namespace muduo {

class Channel;

class EPoller {
 public:
  typedef std::vector<Channel*> ChannelList;
  EPoller(EventLoop* loop);
  ~EPoller();

  Timestamp poll(int timeoutMs, ChannelList& active_channels);
  void fillActiveChannels(int nevents, ChannelList& active_channels);

  void RemoveChannel(Channel* channel);
  void UpdateChannel(Channel* channel);
  void update(int opt, Channel* channel);

  void AssertInLoopThread() {
    loop_->AssertInLoopThread();
  }

 private:
  typedef std::vector<struct epoll_event> EPollFdList;
  typedef std::map<int, Channel*> ChannelMap;

  int epoll_fd_;
  EPollFdList epoll_events_;
  ChannelMap channels_;
  EventLoop* loop_ = nullptr;
};

};

#endif
