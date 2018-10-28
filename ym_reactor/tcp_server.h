#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include <map>
#include "socket.h"
#include "callbacks.h"

namespace muduo {

class Acceptor;
class EventLoop;
class TcpConnection;

class TcpServer {
 public:
  TcpServer(EventLoop* loop, const InetAddress& addr); 
  ~TcpServer();

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_cb_ = cb;
  }

  void SetMessageCallback(const MessageCallback& cb) {
    message_cb_ = cb;
  }

  void Start();

 private:
  void NewConnection(int sockfd, const InetAddress& addr);
  void RemoveConnection(const TcpConnectionPtr& conn);

  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;
  std::unique_ptr<Acceptor> acceptor_;
  std::string conn_name_;
  ConnectionCallback connection_cb_;
  MessageCallback message_cb_;
  int conn_id_ = 0;
  bool start_ = false;
  ConnectionMap conns_;
};

};

#endif

