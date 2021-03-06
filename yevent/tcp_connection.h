#ifndef YEVENT_TCP_CONNECTION_H
#define YEVENT_TCP_CONNECTION_H

#include <vector>
#include "callbacks.h"
#include "socket.h"
#include "channel.h"
#include "inet_address.h"
#include "buffer.h"

namespace yevent {

class EventLoop;

enum TcpState {
  kConnecting = 0,
  kConnected = 1,
  kDisconnecting = 2,
  kDisconnected = 3,
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
  // internal use for tcp_server/tcp_client
  void SetCloseCallback(const CloseCallback& cb) {
    close_cb_ = cb;
  }

  void Connect();
  // called when TcpServer remove this conn from its map
  void Destroy();

  void Send(const std::string& message);
  void Shutdown();

  EventLoop* GetLoop() const { return loop_; }

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
  void SendInLoop(const std::string& message);
  void ShutdownInLoop();
  void SetTcpNoDelay(bool on) {
    socket_->SetTcpNoDelay(true);
  }
  void SetTcpKeepAlive(bool on) {
    socket_->SetTcpKeepAlive(true);
  }

  void SetState(TcpState s) {
    state_ = s;
  }
  void HandleRead();
  void HandleWrite();
  void HandleClose();
  void HandleError();

  EventLoop*loop_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<Socket> socket_;
  std::string conn_name_;
  InetAddress local_addr_;
  InetAddress peer_addr_;
  TcpState state_;

  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  CloseCallback close_cb_;
  Buffer inbuf_;
  Buffer outbuf_;
};

};

#endif
