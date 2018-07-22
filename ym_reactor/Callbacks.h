#ifndef MUDOU_NET_CALLBACK_H
#define MUDOU_NET_CALLBACK_H

#include <functional>
#include <iostream>

namespace muduo {

typedef std::function<void ()> TimeoutCallback;

typedef std::function<void()> EventCallback;


};

#endif

