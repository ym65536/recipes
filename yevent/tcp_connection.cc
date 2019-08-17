#include <unistd.h>
#include "event_loop.h"
#include "logging.h"
#include "tcp_connection.h"

using namespace yevent;

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, 
                             int sockfd, const InetAddress& local_addr, 
                             const InetAddress& peer_addr) :
    loop_(loop), 
    channel_(new Channel(loop_, sockfd)), 
    socket_(new Socket(sockfd)), 
    conn_name_(name), 
    local_addr_(local_addr),
    peer_addr_(peer_addr), 
    state_(kConnecting) {
  LOG_DEBUG << "Tcp Connection[name=" << conn_name_ << ",fd=" << sockfd 
      << ",peer=" << peer_addr_.ToString() << " construct...";
  channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this));
  channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
  channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
  channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "Tcp Connection[name=" << conn_name_ << ",peer=" 
      << peer_addr_.ToString() << " release...";
}

void TcpConnection::Connect() {
  loop_->AssertInLoop();
  assert(state_ == kConnecting);
  SetState(kConnected);
  channel_->EnableReading();
  connection_cb_(shared_from_this());
}

void TcpConnection::HandleRead() {
  size_t nbytes = inbuf_.ReadFd(socket_->sockfd());
  LOG_TRACE << conn_name_ << ":read buffer size=" << nbytes;
  if (nbytes > 0) {
    message_cb_(shared_from_this(), &inbuf_);
  } else if (nbytes == 0) {
    HandleClose();
  } else {
    LOG_SYSERR << "TcpConnection:HandleRead";
    HandleError();
  }
}

void TcpConnection::HandleWrite() {

}

void TcpConnection::HandleClose() {
  loop_->AssertInLoop();
  LOG_ERROR << "Handle close. state_=" << state_;
  close_cb_(shared_from_this());
}

void TcpConnection::HandleError() {
  int err = sockets::GetSocketError(channel_->Fd());
  LOG_ERROR << "handleError [" << conn_name_ << "] - SO_ERROR = " << err << " " 
      << strerror_tl(err);
}

// 生命周期最后调用函数
void TcpConnection::Destroy() {
  loop_->AssertInLoop();
  assert(state_ == kConnected);
  SetState(kDisconnected);
  connection_cb_(shared_from_this());
  loop_->RemoveChannel(channel_.get());
}

