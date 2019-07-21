#include "event_loop.h"
#include "event_loop_thread.h"
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

void RunInThread() {
  printf("runInThread(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
}

int main() {
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  EventLoopThread loopThread;
  EventLoop* loop = loopThread.StartLoop();
  loop->RunInLoop(RunInThread);
  sleep(1);
  loop->RunAfter(2, RunInThread);
  sleep(3);
  loop->Quit();

  printf("exit main().\n");
}

