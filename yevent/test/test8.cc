#include "event_loop.h"
#include "acceptor.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "base/thread.h"
#include "logging.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

using namespace yevent;

void OnConnection(const TcpConnectionPtr& conn) {
  if (conn->IsConnected()) {
    LOG_DEBUG << "Recv New Connection " << conn->ConnName();
  } else {
    LOG_DEBUG << "Release New Connection " << conn->ConnName();
  }
}

void OnMessage(const TcpConnectionPtr& conn, const char* buf, int nbytes) {
  LOG_DEBUG << "Recv conn=" << conn->ConnName() << ",msg=" << std::string(buf, nbytes);
}

int main() {
  EventLoop loop;
  InetAddress addr("127.0.0.1", 9981);
  TcpServer server(&loop, addr);
  server.SetConnectionCallback(OnConnection);
  server.SetMessageCallback(OnMessage);
  server.Start();

  loop.Loop();
}

