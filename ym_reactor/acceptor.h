#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include <datetime/Timestamp.h>
#include <Channel.h>
#include "inet_address.h"
#include "socket.h"
#include "callbacks.h"

namespace muduo {

class EventLoop;

class Acceptor {
 public:
  Acceptor(EventLoop* loop, const InetAddress& addr);
  ~Acceptor();

  void SetConnectionCallback(const NewConnectionCallback& cb) {
    cb_ = cb;
  }

  void Listen();

  bool is_listenning() const { return listenning_; }

 private:
  void HandleRead();

  EventLoop* loop_;
  Socket socket_;
  Channel accept_channel_;
  NewConnectionCallback cb_;
  bool listenning_ = false;
};

};

#endif

