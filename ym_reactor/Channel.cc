#include "Channel.h"
#include "EventLoop.h"
#include "logging/Logging.h"
#include <assert.h>

using namespace muduo;

Channel::~Channel() {
}

void Channel::updateChannel() {
  loop_->updateChannel(this);
}

void Channel::handleEvent() {
  if (revents_ & EPOLLERR) {
    error_callback_();
  } else if (revents_ & (EPOLLIN | EPOLLPRI)) {
    read_callback_();
  } else if (revents_ & (EPOLLOUT)) {
    write_callback_();
  } else {
    LOG_ERROR << "Unknown events: " << revents_;
  }
}

