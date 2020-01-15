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

int g_times = 0;
EventLoop* g_loop;

void printTid() {
  printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  printf("now %s\n", Timestamp::now().toString().c_str());
}

int main() {
  printTid();
  EventLoop loop;
  g_loop = &loop;

  auto print = [](const char* msg) -> std::function<void()> {
    return [msg]() {
      g_times ++;
      printf("<%d>msg %s %s\n", g_times, Timestamp::now().toString().c_str(), msg);
      if (g_times == 20) {
        g_loop->Quit();
      }
    };
  };
  print("main")();
  loop.RunAfter(1, print("once1"));
  loop.RunAfter(1.5, print("once1.5"));
  loop.RunAfter(2.5, print("once2.5"));
  loop.RunAfter(3.5, print("once3.5"));
  loop.RunEvery(2, print("every2"));
  loop.RunEvery(3, print("every3"));
  loop.Loop();
  print("main loop exits");
  sleep(1);
}
