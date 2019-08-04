#ifndef YEVENT_TCP_CONNECTION_H
#define YEVENT_TCP_CONNECTION_H

#include <vector>
#include "callbacks.h"
#include "socket.h"
#include "channel.h"
#include "inet_address.h"

namespace yevent {

class EventLoop;

enum TcpState {
  kConnecting = 0,
  kConnected = 1,
  kDisconnected = 2,
  kDisconnecting = 3,
};

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name, int sockfd, 
                const InetAddress& local_addr, const InetAddress& peer_addr);
  ~TcpConnection();

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_cb_ = cb;
  }
  void SetMessageCallback(const MessageCallback& cb) {
    message_cb_ = cb;
  }

  void Connect();
  void HandleRead();

  bool IsConnected() {
    return state_ == kConnected;
  }
  std::string ConnName() {
    return conn_name_;
  }
  InetAddress LocalAddr() {
    return local_addr_;
  }
  InetAddress PeerAddr() {
    return peer_addr_;
  }

 private:
  void SetState(TcpState s) {
    state_ = s;
  }

  EventLoop*loop_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<Socket> socket_;
  std::string conn_name_;
  InetAddress local_addr_;
  InetAddress peer_addr_;
  TcpState state_;

  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
};

};

#endif
