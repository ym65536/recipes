#include "socket.h"
#include "inet_address.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  // bzero
#include "logging/Logging.h"

using namespace muduo;

Socket::Socket(int fd): sockfd_(fd) {
}

Socket::Socket(): sockfd_(sockets::Socket()) {
}

Socket::~Socket() {
  sockets::Close(sockfd_);
}

void Socket::Bind(const InetAddress& addr) {
  sockets::Bind(sockfd_, addr.GetSockAddrInet());
}

void Socket::Listen() {
  sockets::Listen(sockfd_);
}

int Socket::Accept(InetAddress& peeraddr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = sockets::Accept(sockfd_, addr);
  if (connfd >= 0) {
    peeraddr.SetSockAddrInet(addr);
  }
  return connfd;
}

void Socket::ReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof optval);
}

void Socket::ShutdownWrite()
{
  if (::shutdown(sockfd_, SHUT_WR) < 0)
  {
    LOG_SYSERR << "sockets::shutdownWrite";
  }
}
