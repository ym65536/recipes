#ifndef YEVENT_EPOLLER_H
#define YEVENT_EPOLLER_H

#include <sys/epoll.h>
#include <map>
#include <vector>

namespace yevent {

class Channel;

const static int INIT_EPOLL_SIZE = 1024;
const static uint32_t EPOLL_FD_SIZE = 65536;

typedef std::vector<struct epoll_event> EPollEventList;
typedef std::map<int, Channel*> ChannelMap;

class EPoller {
 public:
  EPoller(EventLoop* loop);
  ~EPoller();

  void UpdateChannel(Channel* channel);
  void Poll(ChannelList* channels, int TimeoutMs);

 private:
  void EventAdd(Channel* channel);
  void EventMod(Channel* channel);
  void EventDel(Channel* channel);

  EventLoop* loop_;
  int epoll_fd_;
  EPollEventList epoll_events_;
  ChannelMap channels_;
};

};

#endif

