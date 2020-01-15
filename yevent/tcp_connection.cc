#include <unistd.h>
#include <errno.h>
#include<sys/types.h>
#include<sys/socket.h>
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
  channel_->SetReadCallback([this]{
    HandleRead();
  });
  channel_->SetWriteCallback([this]{ 
    HandleWrite(); 
  });
  channel_->SetCloseCallback([this]{
    HandleClose();
  });
  channel_->SetErrorCallback([this]{ 
    HandleError();
  });
  SetTcpNoDelay(true);
  SetTcpKeepAlive(true);
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
  loop_->AssertInLoop();
  LOG_TRACE << "Handle write. state_=" << state_;
  if (!channel_->MonitorWrite())  {
    LOG_ERROR << "Do not monitor write event.";
    return;
  }
  size_t nbytes = ::write(channel_->Fd(), outbuf_.Peek(), outbuf_.ReadableBytes());
  if (nbytes > 0) {
    outbuf_.Retrieve(nbytes);
    if (outbuf_.ReadableBytes() == 0) {
      channel_->DisableWriting();
      if (state_ == kDisconnecting) {
        ShutdownInLoop();
      }
    } else {
      LOG_INFO << "I am going to write more data...";
    }
  } else {
    LOG_SYSERR << "HandleWrite";
  }
}

void TcpConnection::HandleClose() {
  loop_->AssertInLoop();
  LOG_ERROR << "Handle close. state=" << state_;
  assert(state_ == kConnected || state_ == kDisconnecting);
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
  assert(state_ == kConnected || state_ == kDisconnecting);
  SetState(kDisconnected);
  connection_cb_(shared_from_this());
  loop_->RemoveChannel(channel_.get());
}

void TcpConnection::Shutdown() {
  if (state_ == kConnected) {
    SetState(kDisconnecting);
    loop_->RunInLoop([self = shared_from_this()] {
        self->ShutdownInLoop();
    });
  }
}

void TcpConnection::ShutdownInLoop() {
  loop_->AssertInLoop();
  if (channel_->MonitorWrite()) {
    LOG_WARN << "In loop thread, don not shutdown.";
    return;
  }
  socket_->ShutdownWrite();
}

void TcpConnection::Send(const std::string& message) {
  if (state_ != kConnected) {
    LOG_ERROR << "Not connect. do not send message.";
    return;
  }
  if (loop_->IsInLoop()) {
    SendInLoop(message);
  } else {
    loop_->RunInLoop([this, &message] {
        SendInLoop(message);
      });
  }
}

void TcpConnection::SendInLoop(const std::string& message) {
  loop_->AssertInLoop();
  size_t nbytes = 0;
  if (!channel_->MonitorWrite() && outbuf_.ReadableBytes() == 0) {
    nbytes = ::write(channel_->Fd(), message.data(), message.size());
    if (nbytes >= 0) {
      if (nbytes < message.size()) {
        LOG_DEBUG << "I am going to write more data...";
      }
    } else {
      nbytes = 0;
      if (errno != -EAGAIN) {
        LOG_SYSERR << "TcpConnection::SendInLoop";
      }
    }
  }

  if (nbytes < message.size()) {
    outbuf_.Append(message.data() + nbytes, message.size() - nbytes);
    if (!channel_->MonitorWrite()) {
      channel_->EnableWriting();
    }
  }
}


