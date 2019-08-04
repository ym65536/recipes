#ifndef YEVENT_TCP_SERVER_H
#define YEVENT_TCP_SERVER_H

#include <vector>
#include "socket.h"
#include "channel.h"
#include "inet_address.h"
#include "callbacks.h"

namespace yevent {

class EventLoop;

class TcpServer {
 public:
  TcpServer(EventLoop* loop, const InetAddress& addr);
  ~TcpServer();

  bool IsStart() {
    return started_;
  }

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_cb_ = cb;
  }

  void SetMessageCallback(const MessageCallback& cb) {
    message_cb_ = cb;
  }

  void Start();

 private:
  void NewConnection(int sockfd, const InetAddress& peer_addr);
  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop*loop_;
  std::unique_ptr<Acceptor> acceptor_;
  std::string name_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  bool started_;
  uint64_t conn_id_;
  ConnectionMap connections_;
};

};

#endif
