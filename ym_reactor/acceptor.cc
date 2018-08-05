#include "acceptor.h"
#include "logging/Logging.h"
#include "EventLoop.h"
#include <datetime/Timestamp.h>
#include <algorithm>

using namespace std;
using namespace muduo;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr) :
  loop_(loop),
  socket_(),
  socket_channel_(loop, socket_.sockfd()),
  listenning_(false) {
  socket_.ReuseAddr(true);
  socket_.Bind(addr);
  socket_channel_.setReadCallback(std::bind(&Acceptor::HandleRead(), this));
}

Acceptor::~Acceptor() {

}

void Acceptor::Listen() {
  loop_->assertInLoopThread();
  listenning_ = true;
  socket_.Listen();
  socket_channel_.enableReading();
}

void Acceptor::HandleRead() {
  loop_->assertInLoopThread();
  InetAddress addr;
  int connfd = socket_.Accept(&addr);
  if (connfd >= 0) {
    if (cb_) {
      cb_(client_fd, addr);
    } else {
      LOG_ERROR << "no connection callback, close connection.";
      sockets::Close(connfd);
    }
  } else {
    LOG_ERROR << "Accept fail. ret=" << connfd;
  }
}
