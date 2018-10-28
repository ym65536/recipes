#ifndef MUDUO_NET_SOCKET_H
#define MUDUO_NET_SOCKET_H

#include "logging/Logging.h"
#include <sys/epoll.h>
#include <string>

namespace muduo {

class InetAddress;

class Socket {
 public:
  explicit Socket(int fd);
  Socket();
  ~Socket();

  void Bind(const InetAddress& addr);
  void Listen();
  int Accept(InetAddress& peeraddr);
  void ReuseAddr(bool on);
  void ShutdownWrite();

  int sockfd() const { return sockfd_; }
 private:
  int sockfd_;
};

};

#endif
