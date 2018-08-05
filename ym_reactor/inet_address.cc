#include "InetSocket.h"
#include <errno.h>
#include <fcntl.h>

using namespace muduo;
using namespace std;

InetAddress::InetAddress(const std::string& ip, int port) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &(addr_.sin_addr)) <= 0)
  {
    LOG_SYSERR << "sockets::fromHostPort";
  }
}

InetAddress::~InetAddress() {
}

string InetAddress::toHostPort() {
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET, &addr_.sin_addr, host, sizeof host);
  uint16_t port = networkToHost16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);

  return string(buf);
}

int sockets::Socket() {
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 
      IPPROTO_TCP);
  if (sockfd < 0) {
    LOG_SYSFATAL << "sockets::createNonblockingOrDie";
  }
  return sockfd;
}

void sockets::Bind(int sockfd, const struct sockaddr_in& addr ) {
  int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret < 0) {
    LOG_SYSFATAL << "Bind";
  }
}

void sockets::Listen(int sockfd) {
  int ret = listen(fd, BACKLOG);
  if (ret < 0) {
    LOG_SYSFATAL << "Listen";
  }
}

void sockets::Accept(int sockfd, struct sockaddr_in& addr) {
  socklen_t addrlen = sizeof(addr);
  int connfd = accept4(sockfd, (struct sockaddr* )&addr, sizeof(addr), 
      SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    LOG_SYSERR << "accept";
  }
  return connfd;
}

void sockets::Close(int sockfd) {
  if (close(sockfd) < 0) {
    LOG_SYSERR << "close";
  }
}
