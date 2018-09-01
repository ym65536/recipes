#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include "logging/Logging.h"
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

  void HandleEvent();


  void SetReadCallback(const EventCallback& cb) {
    read_callback_ = cb;
  }
  void SetWriteCallback(const EventCallback& cb) {
    write_callback_ = cb;
  }
  void SetErrorCallback(const EventCallback& cb) {
    error_callback_ = cb;
  }
  void SetCloseCallback(const EventCallback& cb) {
    close_callback_ = cb;
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

  void EnableReading() {
    events_ |= READ_EVENT | EPOLLET; // edge trigger?
//events_ |= READ_EVENT  // level trigger?
    LOG_INFO << "Read event=" << events_;
    UpdateChannel();
  }
  
 private:
  void UpdateChannel();
  
  EventLoop* loop_ = nullptr;
  int fd_ = -1;
  int events_ = 0;
  int revents_ = 0;
  bool event_handling_ = false;

  EventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback close_callback_;
  EventCallback error_callback_;
};

};

#endif
