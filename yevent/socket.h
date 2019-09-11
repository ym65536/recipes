#ifndef YEVENT_NET_SOCKET_H
#define YEVENT_NET_SOCKET_H

#include <sys/epoll.h>
#include <string>

namespace yevent {

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
  void SetTcpNoDelay(bool on);
  void SetTcpKeepAlive(bool on);

  int sockfd() const { return sockfd_; }
 private:
  int sockfd_;
};

};

#endif
