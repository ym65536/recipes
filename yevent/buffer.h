#ifndef YEVENT_NET_BUFFER_H
#define YEVENT_NET_BUFFER_H

#include <algorithm>
#include <string>
#include <vector>
#include <assert.h>

namespace yevent {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  Buffer()
    : buffer_(kCheapPrepend + kInitialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend) {
    assert(ReadableBytes() == 0);
    assert(WritableBytes() == kInitialSize);
    assert(PrependableBytes() == kCheapPrepend);
  }

  // default copy-ctor, dtor and assignment are fine

  void Swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }

  size_t ReadableBytes() const
  { return writerIndex_ - readerIndex_; }

  size_t WritableBytes() const
  { return buffer_.size() - writerIndex_; }

  size_t PrependableBytes() const
  { return readerIndex_; }

  const char* Peek() const
  { return Begin() + readerIndex_; }

  // retrieve returns void, to prevent
  // string str(retrieve(ReadableBytes()), ReadableBytes());
  // the evaluation of two functions are unspecified
  void Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readerIndex_ += len;
  }

  void RetrieveUntil(const char* end) {
    assert(Peek() <= end);
    assert(end <= BeginWrite());
    Retrieve(end - Peek());
  }

  void RetrieveAll() {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
  }

  std::string RetrieveAsString() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
  }

  void Append(const std::string& str) {
    Append(str.data(), str.length());
  }

  void Append(const char* /*restrict*/ data, size_t len) {
    EnsureWritableBytes(len);
    std::copy(data, data+len, BeginWrite());
    HasWritten(len);
  }

  void Append(const void* /*restrict*/ data, size_t len) {
    Append(static_cast<const char*>(data), len);
  }

  void EnsureWritableBytes(size_t len) {
    if (WritableBytes() < len) {
      MakeSpace(len);
    }
    assert(WritableBytes() >= len);
  }

  char* BeginWrite()
  { return Begin() + writerIndex_; }

  const char* BeginWrite() const
  { return Begin() + writerIndex_; }

  void HasWritten(size_t len)
  { writerIndex_ += len; }

  void Prepend(const void* /*restrict*/ data, size_t len) {
    assert(len <= PrependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, Begin()+readerIndex_);
  }

  void Shrink(size_t reserve) {
   std::vector<char> buf(kCheapPrepend + ReadableBytes() + reserve);
   std::copy(Peek(), Peek()+ReadableBytes(), buf.begin()+kCheapPrepend);
   buf.swap(buffer_);
  }

  /// Read data directly into buffer.
  /// It may implement with readv(2)
  ssize_t ReadFd(int fd);

  const char* Data() const {
    return buffer_.data();
  }

 private:

  char* Begin()
  { return &*buffer_.begin(); }

  const char* Begin() const
  { return &*buffer_.begin(); }

  void MakeSpace(size_t len) {
    if (WritableBytes() + PrependableBytes() < len + kCheapPrepend) {
      buffer_.resize(writerIndex_+len);
    } else {
      // move readable data to the front, make space inside buffer
      assert(kCheapPrepend < readerIndex_);
      size_t readable = ReadableBytes();
      std::copy(Begin() + readerIndex_,
                Begin() + writerIndex_,
                Begin() + kCheapPrepend);
      readerIndex_ = kCheapPrepend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == ReadableBytes());
    }
  }

 private:
  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;
};

}

#endif  // YEVENT_NET_BUFFER_H
