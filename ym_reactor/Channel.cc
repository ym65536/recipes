#include "Channel.h"
#include "EventLoop.h"
#include "logging/Logging.h"
#include <assert.h>

using namespace muduo;

Channel::Channel(EventLoop* loop, int fd):
  loop_(loop), fd_(fd) {
}

Channel::~Channel() {
  assert(event_handling_ == false);
}

void Channel::UpdateChannel() {
  loop_->UpdateChannel(this);
}

void Channel::HandleEvent() {
  event_handling_ = true;
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    close_callback_();
  } else if (revents_ & EPOLLERR) {
    error_callback_();
  } else if (revents_ & (EPOLLIN | EPOLLPRI)) {
    read_callback_();
  } else if (revents_ & (EPOLLOUT)) {
    write_callback_();
  } else {
    LOG_ERROR << "Unknown events: " << revents_;
  }
  event_handling_ = false;
}

