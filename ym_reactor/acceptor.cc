#include "acceptor.h"
#include "logging/Logging.h"
#include "event_loop.h"
#include <datetime/Timestamp.h>
#include <algorithm>

using namespace std;
using namespace muduo;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr) :
  loop_(loop),
  socket_(),
  accept_channel_(loop, socket_.sockfd()),
  listenning_(false) {
  socket_.ReuseAddr(true);
  socket_.Bind(addr);
  accept_channel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {

}

void Acceptor::Listen() {
  loop_->AssertInLoopThread();
  listenning_ = true;
  socket_.Listen();
  accept_channel_.EnableReading();
}

void Acceptor::HandleRead() {
  loop_->AssertInLoopThread();
  InetAddress addr;
  int connfd = socket_.Accept(addr);
  if (connfd >= 0) {
    if (cb_) {
      cb_(connfd, addr);
    } else {
      LOG_ERROR << "no connection callback, close connection.";
      sockets::Close(connfd);
    }
  } else {
    LOG_ERROR << "Accept fail. ret=" << connfd;
  }
}
