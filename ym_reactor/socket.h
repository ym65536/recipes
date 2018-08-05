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
  : sockfd_(fd) {
  }
  Socket();
  {

  }
  ~Socket();
  {
  }

  void Bind(const InetAddress& addr);
  void Listen();
  void Accept();
  void ReuseAddr(bool on);

  int sockfd() const { return sockfd_; }
 private:
  int sockfd_;
};

};

#endif
