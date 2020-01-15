#include "event_loop.h"
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

using namespace yevent;

EventLoop* g_loop = nullptr;

int main()
{
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  EventLoop loop;
  g_loop = &loop;
  int timerfd = timerfd_create(CLOCK_REALTIME, O_NONBLOCK | O_CLOEXEC);
  assert(timerfd > 0);
  Channel channel(g_loop, timerfd);
  channel.SetReadCallback([] {
    printf("threadFunc(): pid = %d, tid = %d\n",
         getpid(), yevent::CurrentThread::tid());
    g_loop->Quit(); });
  channel.EnableReading();

  struct itimerspec ts;
  memset(&ts, 0 , sizeof(ts));
  ts.it_value.tv_sec = 3;
  timerfd_settime(timerfd, 0, &ts, nullptr);

  loop.Loop();
}
