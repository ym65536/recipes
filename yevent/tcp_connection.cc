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
  char buffer[65536] = {0};
  size_t nbytes = ::read(socket_->sockfd(), buffer, sizeof buffer);
  LOG_TRACE << conn_name_ << ":read buffer size=" << nbytes;
  message_cb_(shared_from_this(), buffer, nbytes);
}

