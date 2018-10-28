#ifndef MUDUO_NET_TCP_CONNECTION_H
#define MUDUO_NET_TCP_CONNECTION_H

#include "tcp_connection.h"
#include <memory>
#include <inet_address.h>
#include "socket.h"
#include "Callbacks.h"
#include "buffer.h"

namespace muduo {

class EventLoop;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop*, const std::string&, int, 
      const InetAddress&, const InetAddress&);
  ~TcpConnection();

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_cb_ = cb;
  }

  void SetMessageCallback(const MessageCallback& cb) {
    message_cb_ = cb;
  }

  void SetCloseCallback(const CloseCallback& cb) {
    close_cb_ = cb;
  }

  // start connection
  void Connect();
  // destroy connection
  void Destroy();

  void Send(const std::string& message);
  
  void Shutdown();

  bool IsConnected() {
    return state_ == kConnected;
  }

  std::string ConnName() {
    return conn_name_;
  }

  InetAddress PeerAddr() {
    return peer_addr_;
  }

  InetAddress LocalAddr() {
    return local_addr_;
  }

 private:
  enum TcpState {
    kConnecting = 0,
    kConnected = 1,
    kDisconnected = 2,
    kDisconnecting = 3,
  };

  void SetState(TcpState s) {
    state_ = s;
  }

  void HandleRead();
  void HandleWrite();
  void HandleClose();
  void HandleError();
  void SendInLoop(const std::string& message);
  void ShutdownInLoop();

  EventLoop* loop_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<Socket> socket_;
  std::string conn_name_;
  InetAddress peer_addr_;
  InetAddress local_addr_;
  TcpState state_;

  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  CloseCallback close_cb_;
  Buffer inbuf_;
  Buffer outbuf_;
};

}

#endif
