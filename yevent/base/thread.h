#ifndef YEVENT_BASE_THREAD_H
#define YEVENT_BASE_THREAD_H

#include <atomic>
#include <memory>
#include <pthread.h>

namespace yevent {

class Thread  {
 public:
  typedef std::function<void ()> ThreadFunc;

  explicit Thread(const ThreadFunc&, const std::string& name = std::string());
  ~Thread();

  void start();
  void join();

  bool started() const { return started_; }
  // pthread_t pthreadId() const { return pthreadId_; }
  pid_t tid() const { return *tid_; }
  const std::string& name() const { return name_; }

  static int numCreated() { return numCreated_.load(); }

 private:
  bool        started_;
  bool        joined_;
  pthread_t   pthreadId_;
  std::shared_ptr<pid_t> tid_;
  ThreadFunc  func_;
  std::string name_;
  static std::atomic<uint32_t> numCreated_;
};

namespace CurrentThread
{
  pid_t tid();
  const char* name();
  bool isMainThread();
}

}

#endif
