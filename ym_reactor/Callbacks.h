#ifndef MUDOU_NET_CALLBACK_H
#define MUDOU_NET_CALLBACK_H

#include <functional>
#include <iostream>

namespace muduo {

class InetAddress;

typedef std::function<void ()> TimeoutCallback;

typedef std::function<void()> EventCallback;

typedef std::function<void()> Functor;

typedef std::function<void(int fd, const InetAddress& addr)>
  NewConnectionCallback;

};

#endif

