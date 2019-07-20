#ifndef YEVENT_CHANNEL_H
#define YEVENT_CHANNEL_H

#include <sys/epoll.h>
#include <vector>
#include "callbacks.h"

namespace yevent {

class EventLoop;

enum { 
  kNoneEvent = 0,
  kReadEvent = EPOLLIN | EPOLLPRI,
  kWriteEvent = EPOLLOUT,
};

class Channel {
 public:
  Channel(EventLoop* loop, int fd);
  ~Channel();

  void HandleEvent();

  void SetReadCallback(const EventCallback& cb) {
    read_cb_ = cb;
  }
  void SetWriteCallback(const EventCallback& cb) {
    write_cb_ = cb;
  }
  void SetErrorCallback(const EventCallback& cb) {
    error_cb_ = cb;
  }

  int Fd() const {
    return fd_;
  }

  int Events() const {
    return events_;
  }

  void SetActiveEvents(int events) {
    active_events_ = events;
  }

  void EnableReading() {
    events_ |= kReadEvent | EPOLLET; // 采用ET模式
    UpdateEvents();
  }

  bool IsNoneEvent() const {
    return events_ == kNoneEvent;
  }

  EventLoop* GetLoop() const {
    return loop_;
  }

 private:
  void UpdateEvents();

  EventLoop* loop_;
  int fd_;
  int events_;
  int active_events_;

  EventCallback read_cb_;
  EventCallback write_cb_;
  EventCallback error_cb_;
};

typedef std::vector<Channel*> ChannelList;

};

#endif

