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

void print(const char* msg) {
  g_times ++;
  printf("<%d>msg %s %s\n", g_times, Timestamp::now().toString().c_str(), msg);
  if (g_times == 20) {
    g_loop->Quit();
  }
}

int main() {
  printTid();
  EventLoop loop;
  g_loop = &loop;

  print("main");
  loop.RunAfter(1, std::bind(print, "once1"));
  
  loop.RunAfter(1.5, std::bind(print, "once1.5"));
  loop.RunAfter(2.5, std::bind(print, "once2.5"));
  loop.RunAfter(3.5, std::bind(print, "once3.5"));
  loop.RunEvery(2, std::bind(print, "every2"));
  loop.RunEvery(3, std::bind(print, "every3"));

  loop.Loop();
  print("main loop exits");
  sleep(1);
}
