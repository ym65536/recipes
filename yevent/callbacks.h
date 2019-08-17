#ifndef YEVENT_CALLBACKS_H
#define YEVENT_CALLBACKS_H

#include <functional>
#include <memory>

namespace yevent {

class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> EventCallback;

typedef std::function<void()> TimerCallback;

typedef std::function<void()> Functor;

typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;

typedef std::function<void(const TcpConnectionPtr&, Buffer*)> MessageCallback;

typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;

}

#endif

