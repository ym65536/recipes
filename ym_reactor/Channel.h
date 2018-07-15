#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include "thread/Thread.h"
#include <sys/epoll.h>
#include <string>

namespace muduo {

class EventLoop;

typedef std::function<void()> EventCallback;

enum {
  READ_EVENT = EPOLLIN | EPOLLPRI,
  WRITE_EVENT = EPOLLOUT,
  ERROR_EVENT = 0,
};

class Channel {
 public:
  Channel(EventLoop* loop, int fd):
    loop_(loop), fd_(fd) {
    }
  ~Channel();

  void handleEvent();
  void setReadCallback(const EventCallback& cb) {
    read_callback_ = cb;
  }

  void setWriteCallback(const EventCallback& cb) {
    write_callback_ = cb;
  }

  void setErrorCallback(const EventCallback& cb) {
    error_callback_ = cb;
  }
  int fd() const {
    return fd_;
  }

  EventLoop* ownerLoop() {
    return loop_;
  }

  int events() const {
    return events_;
  }

  void setRevents(int events) {
    revents_ = events;
  }

  void enableReading() {
    events_ |= READ_EVENT | EPOLLET; // edge trigger?
//events_ |= READ_EVENT  // level trigger?
    updateChannel();
  }
  
 private:
  void updateChannel();
  
  EventLoop* loop_ = nullptr;
  int fd_ = -1;
  int events_ = 0;
  int revents_ = 0;

  EventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback error_callback_;
};

};

#endif