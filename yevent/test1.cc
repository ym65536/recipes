#include "event_loop.h"
#include "base/thread.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


using namespace yevent;

void threadFunc()
{
  printf("threadFunc(): pid = %d, tid = %d\n",
         getpid(), yevent::CurrentThread::tid());

  EventLoop loop;
  loop.Loop();
}

int main()
{
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  EventLoop loop;

  Thread thread(threadFunc);
  thread.start();

  loop.Loop();
  pthread_exit(NULL);
}
