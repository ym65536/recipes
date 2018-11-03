// copied from muduo/net/tests/TimerQueue_unittest.cc

#include "event_loop.h"

#include <boost/bind.hpp>

#include <stdio.h>

int cnt = 0;
muduo::EventLoop* g_loop;

void printTid()
{
  printf("pid = %d, tid = %d\n", getpid(), muduo::CurrentThread::tid());
  printf("now %s\n", muduo::Timestamp::now().toString().c_str());
}

void print(const char* msg)
{
  ++cnt;
  printf("<%d>msg %s %s\n", cnt, muduo::Timestamp::now().toString().c_str(), msg);
  if (cnt == 20)
  {
    g_loop->quit();
  }
}

int main()
{
  printTid();
  muduo::EventLoop loop;
  g_loop = &loop;

  print("main");
  loop.RunAfter(1, boost::bind(print, "once1"));
  
  loop.RunAfter(1.5, boost::bind(print, "once1.5"));
  loop.RunAfter(2.5, boost::bind(print, "once2.5"));
  loop.RunAfter(3.5, boost::bind(print, "once3.5"));
  loop.RunEvery(2, boost::bind(print, "every2"));
  loop.RunEvery(3, boost::bind(print, "every3"));

  loop.loop();
  print("main loop exits");
  sleep(1);
}
