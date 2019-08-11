#include "logging.h"
#include "channel.h"
#include "event_loop.h"

using namespace yevent;

Channel::Channel(EventLoop* loop, int fd) : 
    loop_(loop), fd_(fd), events_(0), active_events_(0), event_doing_(false) {
  LOG_DEBUG << "Channle construct. fd=" << fd_;
}

Channel::~Channel() {
  LOG_DEBUG << "Channle destruct. fd=" << fd_;
}

void Channel::UpdateEvents() {
  loop_->UpdateChannel(this);
}

void Channel::HandleEvent() {
  event_doing_ = true;
  LOG_DEBUG << "[Chanle " << fd_ << "] event=" << active_events_;
  if (active_events_ & EPOLLERR) {
    LOG_DEBUG << "Recv epoll error.event=" << active_events_;
    error_cb_();
  } else if ((active_events_ & EPOLLHUP) && !(active_events_ & EPOLLIN)) {
    LOG_DEBUG << "Recv epoll close.event=" << active_events_;
    close_cb_();
  } else if (active_events_ & EPOLLIN || active_events_ & EPOLLPRI) {
    LOG_DEBUG << "Recv epoll in.event=" << active_events_;
    read_cb_();
  } else if (active_events_ & EPOLLOUT) {
    LOG_DEBUG << "Recv epoll out.event=" << active_events_;
    write_cb_();
  } else {
    LOG_ERROR << "Unknown epoll events=" << active_events_;
  }
  event_doing_ = false;
}

