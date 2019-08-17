#include "buffer.h"
#include <errno.h>
#include <memory.h>
#include <sys/uio.h>

using namespace yevent;

ssize_t Buffer::ReadFd(int fd) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = WritableBytes();
  vec[0].iov_base = Begin() + writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  const ssize_t n = readv(fd, vec, 2);
  if (n < 0) {
    return n;
  } else if ((size_t)n <= writable) {
    writerIndex_ += n;
  } else {
    writerIndex_ = buffer_.size();
    Append(extrabuf, n - writable);
  }
  return n;
}

