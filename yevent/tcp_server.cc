#include "acceptor.h"
#include "event_loop.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "logging.h"

using namespace yevent;
using namespace std::placeholders;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr) :
    loop_(loop),
    acceptor_(new Acceptor(loop, addr)),
    name_(addr.ToString()),
    started_(false),
    conn_id_(0) {
  acceptor_->SetNewConnectionCallback(
      std::bind(&TcpServer::NewConnection, this, _1, _2));
  LOG_DEBUG << name_ << " tcp server construct..."; 
}

TcpServer::~TcpServer() {
  LOG_DEBUG << name_ << " tcp server release..."; 
}

void TcpServer::Start() {
  loop_->AssertInLoop();
  assert(!started_);
  acceptor_->Listen();
}

void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
  loop_->AssertInLoop();
  std::string conn_name = 
      name_ + "_" + std::to_string(conn_id_) + "_" + peer_addr.ToString();
  LOG_DEBUG << "TcpServer new connect:fd=" << sockfd << ",conn=" << conn_name;
  ++ conn_id_;
  InetAddress local_addr = sockets::GetLocalAddr(sockfd);
  TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop_, conn_name, 
                                                sockfd, local_addr, peer_addr);
  connections_[conn_name] = conn;
  conn->SetConnectionCallback(connection_cb_);
  conn->SetMessageCallback(message_cb_);
  conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, _1));
  conn->Connect();
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->AssertInLoop();
  LOG_DEBUG << "TcpServer remove conn=" << conn->ConnName();
  connections_.erase(conn->ConnName());
  loop_->QueueInLoop(std::bind(&TcpConnection::Destroy, conn));
}

