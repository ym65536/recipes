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
  int retcode = 0;
  size_t nbytes = inbuf_.ReadFd(socket_->sockfd(), &retcode);
  LOG_DEBUG << "Recv msg size=" << nbytes << ", content=" << inbuf_.Peek();
  if (nbytes > 0) {
    message_cb_(shared_from_this(), &inbuf_);
  } else if (nbytes == 0) {
    HandleClose();
  } else {
    errno = retcode;
    LOG_SYSERR << "TcpConnection: HandleRead";
    HandleError();
  }
}

void TcpConnection::HandleWrite(){
  LOG_INFO << "HandleWrite called.";
  loop_->AssertInLoopThread();
  if (!channel_->CanWrite()) {
    LOG_ERROR << "Connection is down, no more write.";
    return;
  }
  ssize_t nbytes = ::write(channel_->fd(), 
                           outbuf_.Peek(),
                           outbuf_.ReadableBytes());
  if (nbytes < 0) {
    LOG_SYSERR << "TcpConnection HandleWrite";
    return;
  }
  outbuf_.Retrieve(nbytes);
  if (outbuf_.ReadableBytes() == 0) {
    channel_->DisableWrite();
    if (state_ == kDisconnecting) {
      ShutdownInLoop();
    }
  } else {
    LOG_WARN << "Going to write more data. nbytes=" << nbytes 
        << ", left=" << outbuf_.ReadableBytes();
  }
}

void TcpConnection::HandleClose() {
  assert(state_ == kConnected);
  LOG_DEBUG << "My Channle id=" << channel_->fd();
  channel_->DisableAll();
  close_cb_(shared_from_this());
}

void TcpConnection::HandleError() {
  int err = sockets::GetSocketError(channel_->fd());
  LOG_ERROR << "handleError [" << conn_name_ << "] - SO_ERROR = " << err << " " 
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

  // FixMe: need call queueInLoop, otherwise channle_ will be release
  // before handleEvent finish.
  loop_->RemoveChannel(channel_.get());
}

void TcpConnection::Send(const std::string& message) {
  assert(state_ == kConnected);
  loop_->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, message));
}

void TcpConnection::SendInLoop(const std::string& message) {
  loop_->AssertInLoopThread();
  ssize_t nbytes = 0;
  if (!channel_->CanWrite() && outbuf_.ReadableBytes() == 0) {
    nbytes = ::write(channel_->fd(), message.data(), message.size());
    if (nbytes >= 0) {
      if ((uint32_t)nbytes < message.size()) {
        LOG_WARN << "Going to write more data. nbytes=" << nbytes 
            << ",message_size=" << message.size();
      }
    } else {
      nbytes = 0;
      if (errno != EAGAIN) {
        LOG_SYSERR << "TcpConnection::SendInLoop";
      }
    }
  }

  assert(nbytes >= 0);
  if ((uint32_t)nbytes < message.size()) {
    outbuf_.Append(message.data() + nbytes, message.size() - nbytes);
    if (!channel_->CanWrite()) {
      channel_->EnableWriting();
    }
  }
}

void TcpConnection::Shutdown() {
  assert(state_ == kConnected);
  SetState(kDisconnecting);
  loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
}

void TcpConnection::ShutdownInLoop() {
  loop_->AssertInLoopThread();
  if (!channel_->CanWrite()) {
    socket_->ShutdownWrite();
  } else {
    LOG_ERROR << "Writing...Can not shutdown";
  }
}
