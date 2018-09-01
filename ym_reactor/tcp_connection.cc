#include "logging/Logging.h"
#include "EventLoop.h"
#include "inet_address.h"
#include "tcp_connection.h"

using namespace std;
using namespace muduo;
using namespace std::placeholders;

TcpConnection::TcpConnection(EventLoop* loop, 
    const std::string& name, int sockfd, 
    const InetAddress& peer_addr, const InetAddress& local_addr) :
    loop_(loop), 
    channel_(new Channel(loop_, sockfd)), 
    socket_(new Socket(sockfd)), 
    conn_name_(name), 
    peer_addr_(peer_addr), 
    local_addr_(local_addr),
    state_(kConnecting) {
  LOG_DEBUG << "Tcp Connection[name=" << conn_name_ << ", fd=" << sockfd 
    << ", peer=" << peer_addr_.ToHostPort();
  channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this));
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "Tcp Release Connection[name=" << conn_name_ 
    << ", peer=" << peer_addr_.ToHostPort();
}

void TcpConnection::Connect() {
  loop_->AssertInLoopThread();
  assert(state_ == kConnecting);
  SetState(kConnected);
  channel_->EnableReading();
}

void TcpConnection::HandleRead() {
  char buf[0x1000];
  size_t nbytes = ::read(socket_->sockfd(), buf, sizeof(buf));
  if (nbytes > 0) {
    message_cb_(shared_from_this(), buf, nbytes);
  } else if (nbytes == 0) {
    HandleClose();
  } else {
    HandleError();
  }
}

void TcpConnection::HandleWrite(){
  LOG_INFO << "HandleWrite called.";
}

void TcpConnection::HandleClose() {
  assert(state_ == kConnected);
  channel_->DisableAll();
  close_cb_(shared_from_this());
}

void TcpConnection::HandleError() {
  int err = sockets::GetSocketError(channel_->fd());
  LOG_ERROR << "handleError [" << name_ << "] - SO_ERROR = " << err << " " 
      << strerror_tl(err);
}

/*
  Destroy is last function before this tcp connection destroyed. 
 */
void TcpConnection::Destroy() {
  assert(state_ == kConnected);
  SetState(kDisconnected);
  channel_->DisableAll();
  connection_cb_(shared_from_this());

  loop_->RemoveChannel(channel_);
}

