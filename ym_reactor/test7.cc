#include <iostream>
#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"

using namespace std;
using namespace muduo;

void OnConnection(int sockfd, const InetAddress& peerAddr)
{
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.ToHostPort().c_str());
  ::write(sockfd, "How are you?\n", 13);
  sockets::Close(sockfd);
}

int main(void) {
  cout << "main thread=" << getpid();

  InetAddress addr("127.0.0.1", 9981);
  EventLoop loop;

  Acceptor acceptor(&loop, addr);
  acceptor.SetConnectionCallback(OnConnection);
  acceptor.Listen();

  loop.loop();

  return 0;
}
