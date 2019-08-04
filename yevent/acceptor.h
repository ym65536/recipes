#ifndef YEVENT_ACCEPTOR_H
#define YEVENT_ACCEPTOR_H

#include <vector>
#include "callbacks.h"
#include "socket.h"
#include "channel.h"
#include "inet_address.h"

namespace yevent {

class EventLoop;

typedef std::function<void(int, const InetAddress&)> NewConntionCallback;

class Acceptor {
 public:
  Acceptor(EventLoop* loop, const InetAddress& addr);
  ~Acceptor();

  bool IsListenning() {
    return listenning_;
  }

  void SetNewConnectionCallback(const NewConntionCallback& cb) {
    new_conn_cb_ = cb;
  }

  void Listen();

 private:
  void HandleRead();

  EventLoop*loop_;
  Socket sock_;
  Channel channel_;
  NewConntionCallback new_conn_cb_;
  bool listenning_;
};

};

#endif
