#include "tcp_server.h"
#include "logging/Logging.h"
#include "EventLoop.h"
#include "acceptor.h"
#include "inet_address.h"
#include "tcp_connection.h"

using namespace std::placeholders;
using namespace std;
using namespace muduo;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr) :
  loop_(loop), 
  conn_name_(addr.ToHostPort()) {
  acceptor_.reset(new Acceptor(loop, addr));
  acceptor_->SetConnectionCallback(std::bind(&TcpServer::NewConnection, this, _1,
      _2));
  }

TcpServer::~TcpServer() {
}

void TcpServer::Start() {
  if (!start_) {
    start_ = true;
  }
  loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
}

void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
  loop_->AssertInLoopThread();
  ++ conn_id_;
  std::string conn_name = conn_name_ + std::to_string(conn_id_);
  LOG_DEBUG << "Get new conneciton=" << conn_name << 
    " from " << peer_addr.ToHostPort();
  
  InetAddress local_addr(sockets::GetLocalAddr(sockfd));
  TcpConnectionPtr conn = std::make_shared<TcpConnection>(
      loop_, conn_name, sockfd, peer_addr, local_addr);
  conns_[conn_name] = conn;
  conn->SetConnectionCallback(connection_cb_);
  conn->SetMessageCallback(message_cb_);
  conn->SetCloseCallback(std::bind(&TcpServer::ReomveConnection, this, _1));
  conn->Connect();
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  LOG_DEBUG << "remove conneciton=" << conn->ConnName() << 
    " from " << conn->PeerAddr().ToHostPort();
  conns_.erase(conn->ConnName());
  loop_->QueueInLoop(std::bind(&TcpConnection::Destroy(), conn));
}
