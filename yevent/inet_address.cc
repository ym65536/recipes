#include "inet_address.h"
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "logging.h"

using namespace yevent;
using namespace std;

const static int BACKLOG = 32;

InetAddress::InetAddress(const std::string& ip, int port) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip.c_str(), &(addr_.sin_addr)) <= 0) {
    LOG_SYSERR << "sockets::fromHostPort";
  }
}

InetAddress::InetAddress(const struct sockaddr_in& addr) {
  std::string ip = inet_ntoa(addr.sin_addr);
  InetAddress(ip, addr.sin_port);
}

InetAddress::~InetAddress() {
}

std::string InetAddress::ToString() const {
  char host[INET_ADDRSTRLEN] = "INVALID";
  char buf[100] = {0};
  sockaddr_in addr = addr_;
  ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
  uint16_t port = networkToHost16(addr.sin_port);
  snprintf(buf, 100, "%s:%u", host, port);

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
  int ret = listen(sockfd, BACKLOG);
  if (ret < 0) {
    LOG_SYSFATAL << "Listen";
  }
}

int sockets::Accept(int sockfd, struct sockaddr_in& addr) {
  socklen_t addrlen = sizeof(addr);
  int connfd = accept4(sockfd, (struct sockaddr* )&addr, &addrlen, 
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

struct sockaddr_in sockets::GetLocalAddr(int sockfd) {
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = sizeof(localaddr);
  if (::getsockname(sockfd, reinterpret_cast<sockaddr*>(&localaddr), &addrlen) < 0) {
    LOG_SYSERR << "sockets::getLocalAddr";
  }
  return localaddr;
}

int sockets::GetSocketError(int sockfd) {
  int optval;
  socklen_t optlen = sizeof optval;
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}
