#include "channel.h"
#include "event_loop.h"
#include "logging/Logging.h"
#include <assert.h>

using namespace muduo;

Channel::Channel(EventLoop* loop, int fd):
    loop_(loop), 
    fd_(fd), 
    event_handling_(false) {
}

Channel::~Channel() {
  assert(event_handling_ == false);
  LOG_DEBUG << "Channel release. my fd=" << fd_;
}

void Channel::UpdateChannel() {
  loop_->UpdateChannel(this);
}

void Channel::HandleEvent() {
  event_handling_ = true;
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    LOG_DEBUG << "[Chanle " << fd_ << "] close.";
    close_callback_();
  } else if (revents_ & EPOLLERR) {
    LOG_DEBUG << "[Chanle " << fd_ << "] error.";
    error_callback_();
  } else if (revents_ & (EPOLLIN | EPOLLPRI)) {
    LOG_DEBUG << "[Chanle " << fd_ << "] read.";
    read_callback_();
  } else if (revents_ & (EPOLLOUT)) {
    LOG_DEBUG << "[Chanle " << fd_ << "] write.";
    write_callback_();
  } else {
    LOG_ERROR << "Unknown events: " << revents_;
  }
  LOG_DEBUG << "[Chanle " << fd_ << "] finish.";
  event_handling_ = false;
}

