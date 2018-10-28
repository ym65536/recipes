#ifndef MUDOU_NET_CALLBACK_H
#define MUDOU_NET_CALLBACK_H

#include <functional>
#include <iostream>
#include <memory>

namespace muduo {

class InetAddress;
class TcpConnection;
class Buffer;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void ()> TimeoutCallback;

typedef std::function<void()> EventCallback;

typedef std::function<void()> Functor;

typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;

typedef std::function<void (int sockfd,
                  const InetAddress&)> NewConnectionCallback;

typedef std::function<void (const TcpConnectionPtr&,
                            Buffer* buf)> MessageCallback;

typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;

};

#endif

