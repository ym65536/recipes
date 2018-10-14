#include <memory>
#include <stdio.h>
#include "tcp_server.h"
#include "tcp_connection.h"
#include "EventLoop.h"
#include "inet_address.h"
#include "Callbacks.h"

using namespace muduo;

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->IsConnected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->ConnName().c_str(),
           conn->PeerAddr().ToHostPort().c_str());
  }
  else
  {
    printf("onConnection(): connection [%s] is down\n",
           conn->ConnName().c_str());
  }
}

void onMessage(const TcpConnectionPtr& conn,
               Buffer* buf) {
  printf("onMessage(): received %zd bytes from connection [%s]\n",
         buf->ReadableBytes(), conn->ConnName().c_str());
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  InetAddress addr("127.0.0.1", 9981);
  EventLoop loop;

  muduo::TcpServer server(&loop, addr);
  server.SetConnectionCallback(onConnection);
  server.SetMessageCallback(onMessage);
  server.Start();

  loop.loop();
}
