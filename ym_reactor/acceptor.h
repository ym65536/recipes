#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include <datetime/Timestamp.h>
#include <Channel.h>
#include "InetAddress.h"

namespace muduo {

class EventLoop;

class Acceptor {
 public:
  Acceptor(EventLoop* loop, const InetAddress& addr);
  ~Acceptor();

  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    cb_ = cb;
  }

  bool Listenning() const { return listenning_; }

 private:
  void handleRead();

  EventLoop* loop_;
  Socket socket_;
  Channel acceptor_channel_;
  NewConnectionCallback cb_;
  bool listennig_;
};

};

#endif

