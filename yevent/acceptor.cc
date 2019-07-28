#include "acceptor.h"
#include "event_loop.h"
#include "logging.h"

using namespace yevent;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr) :
    loop_(loop),
    sock_(),
    channel_(loop, sock_.sockfd()),
    listenning_(false) {
  sock_.ReuseAddr(true);
  sock_.Bind(addr);
  channel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {
}

void Acceptor::Listen() {
  loop_->AssertInLoop();
  sock_.Listen();
  channel_.EnableReading();
  listenning_ = true;
  LOG_DEBUG << "Fd=" << sock_.sockfd() << " is listenning...";
}

void Acceptor::HandleRead() {
  loop_->AssertInLoop();
  InetAddress peer_addr("", 0);
  int connfd = sock_.Accept(peer_addr);
  if (connfd > 0) {
    LOG_DEBUG << "accept new sock fd=" << connfd;
    if (new_conn_cb_) {
      new_conn_cb_(connfd, peer_addr);
    } else {
      LOG_ERROR << "Close new accept fd=" << connfd;
      sockets::Close(connfd);
    }
  } else {
    LOG_SYSERR << "connfd=" << connfd;
  }
}
