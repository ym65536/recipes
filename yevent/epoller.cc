#include <unistd.h>
#include "logging.h"
#include "channel.h"
#include "epoller.h"
#include "event_loop.h"

using namespace yevent;

EPoller::EPoller(EventLoop* loop) : loop_(loop) {
  epoll_fd_ = epoll_create(EPOLL_FD_SIZE);
  if (epoll_fd_ < 0) {
    LOG_SYSFATAL << "create epoll fail.";
    exit(EXIT_FAILURE);
  }
  epoll_events_.resize(INIT_EPOLL_SIZE);
  LOG_DEBUG << "Epoller init epoll fd=" << epoll_fd_;
}

EPoller::~EPoller() {
  LOG_DEBUG << "Epoller release epoll fd=" << epoll_fd_;
  ::close(epoll_fd_);
}

void EPoller::Poll(ChannelList* channels, int TimeoutMs) {
  int nevents = epoll_wait(epoll_fd_, epoll_events_.data(), 
                           (int)epoll_events_.size(), TimeoutMs);
  if (nevents > 0) {
    assert((uint32_t)nevents <= epoll_events_.size());
    for (int i = 0; i < nevents; ++ i) {
      auto channel = reinterpret_cast<Channel*>(epoll_events_[i].data.ptr);
      channel->SetActiveEvents(epoll_events_[i].events);
      channels->push_back(channel);
    }
    if ((uint32_t)nevents == epoll_events_.size()) {
      epoll_events_.resize(nevents * 2);
    }
  } else if (nevents == 0) {
    LOG_DEBUG << "Nothing happen. must be timeout.";
  } else {
    LOG_SYSERR << "epoll_fd=" << epoll_fd_ << ". EPoller::poll()";
  } 
}

void EPoller::UpdateChannel(Channel* channel) {
  loop_->AssertInLoop(); 
  int fd = channel->Fd();
  LOG_DEBUG << "channels size=" << channels_.size() << ",fd=" << fd;
  if (channels_.find(fd) == channels_.end()) {
    channels_[fd] = channel;
    EventAdd(channel);
  } else { 
    assert(channels_[fd] == channel);
    if (channel->IsNoneEvent()) {
      EventDel(channel); 
      channels_.erase(fd);
    } else {
      EventMod(channel);
    }
  }
}

void EPoller::EventAdd(Channel* channel) {
  struct epoll_event ev;
  ev.events = channel->Events();
  ev.data.ptr = channel;
  int fd = channel->Fd();
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
    LOG_SYSERR << "epoll add fail. fd=" << fd;
  }
}

void EPoller::EventMod(Channel* channel) {
  struct epoll_event ev;
  ev.events = channel->Events();
  ev.data.ptr = channel;
  int fd = channel->Fd();
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
    LOG_SYSERR << "epoll mod fail. fd=" << fd;
  }
}

void EPoller::EventDel(Channel* channel) {
  int fd = channel->Fd();
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
    LOG_SYSERR << "epoll del fail. fd=" << fd;
  }
}

