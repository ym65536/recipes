#include "event_loop.h"
#include "acceptor.h"
#include "base/thread.h"
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

void OnNewConnect(int sockfd, const InetAddress& peer) {
  printf("OnNewConnect(): recv connfd=%d from %s\n", sockfd, peer.ToString().c_str());
  ::write(sockfd, "How are you?", 13);
  sockets::Close(sockfd);
}

int main() {
  EventLoop loop;
  InetAddress addr("127.0.0.1", 9981);
  Acceptor acceptor(&loop, addr);
  acceptor.SetNewConnectionCallback(OnNewConnect);
  acceptor.Listen();

  loop.Loop();
}
