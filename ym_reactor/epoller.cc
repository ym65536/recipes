#include "epoller.h"
#include "channel.h"
#include "thread/Thread.h"
#include "logging/Logging.h"
#include <sys/epoll.h>
#include <string>

using namespace muduo;

const static int INIT_EPOLL_SIZE = 1024;

EPoller::EPoller(EventLoop* loop) :
    loop_(loop) {
  epoll_fd_ = epoll_create(EPOLL_FD_SIZE);
  if (epoll_fd_ < 0) {
    LOG_SYSFATAL << "create epoll fail.";
  }
  epoll_events_.resize(INIT_EPOLL_SIZE);
}

EPoller::~EPoller() {
  ::close(epoll_fd_);
}

Timestamp EPoller::poll(int timeoutMs, ChannelList& active_channels) {
  int nevents = ::epoll_wait(epoll_fd_, 
      epoll_events_.data(), 
      static_cast<int>(epoll_events_.size()),
      timeoutMs);
  LOG_DEBUG << "EPoll recv events size=" << nevents;
  Timestamp now(Timestamp::now());
  if (nevents > 0) {
    LOG_DEBUG << "epoll " << nevents << " hannped.";
    fillActiveChannels(nevents, active_channels);
    if (nevents >= (int)epoll_events_.size()) {
      epoll_events_.resize(nevents * 2);
    }
  } else if (nevents == 0) {
    LOG_TRACE << "Nothing happen. must be timeout.";
  } else {
    LOG_SYSERR << "EPoller::poll()";
  }
  return now;
}

void EPoller::fillActiveChannels(int nevents, ChannelList& active_channels) {
  assert(nevents <= (int)epoll_events_.size());
  for (int i = 0; i < nevents; ++ i) {
    Channel* channel = 
      reinterpret_cast<Channel*>(epoll_events_[i].data.ptr);
    channel->setRevents(epoll_events_[i].events);
    active_channels.push_back(channel);
  }
}

void EPoller::UpdateChannel(Channel* channel) {
  AssertInLoopThread();
  int fd = channel->fd();
  LOG_DEBUG << "channels size = " << channels_.size() << ", fd=" << fd;
  if (channels_.find(fd) == channels_.end()) {
    channels_[fd] = channel;
    update(EPOLL_CTL_ADD, channel);
  } else {
    assert(channels_[fd] == channel);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPoller::RemoveChannel(Channel* channel) {
  AssertInLoopThread();
  int fd = channel->fd();
  LOG_DEBUG << "Remove channel, fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  channels_.erase(fd);
  update(EPOLL_CTL_DEL, channel);
}

void EPoller::update(int opt, Channel* channel) {
  struct epoll_event ev;
  ev.events = channel->events();
  ev.data.ptr = channel;
  int fd = channel->fd();
  if (epoll_ctl(epoll_fd_, opt, fd, &ev) < 0) {
    LOG_SYSERR << "epoll ctl fail. fd=" << fd << ", opt=" << opt;
  }
}
