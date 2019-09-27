#include "acceptor.h"
#include "event_loop.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "event_loop_thread_pool.h"
#include "logging.h"

using namespace yevent;
using namespace std::placeholders;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr) :
    loop_(loop),
    acceptor_(new Acceptor(loop, addr)),
    thread_pool_(new EventLoopThreadPool(loop)),
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

void TcpServer::SetThreadNum(int thread_num) {
  assert(thread_num >= 0);
  thread_pool_->SetThreadNum(thread_num);
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
  EventLoop* io_loop = thread_pool_->GetNextLoop();
  LOG_TRACE << "Select loop = " << io_loop;
  TcpConnectionPtr conn = std::make_shared<TcpConnection>(io_loop, conn_name, 
                                                sockfd, local_addr, peer_addr);
  connections_[conn_name] = conn;
  conn->SetConnectionCallback(connection_cb_);
  conn->SetMessageCallback(message_cb_);
  conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, _1));
  conn->Connect();
  io_loop->RunInLoop(std::bind(&TcpConnection::Connect, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn) {
  loop_->AssertInLoop();
  LOG_DEBUG << "TcpServer remove conn=" << conn->ConnName();
  connections_.erase(conn->ConnName());
  EventLoop* io_loop = conn->GetLoop();
  io_loop->QueueInLoop(std::bind(&TcpConnection::Destroy, conn));
}

