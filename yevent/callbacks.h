#ifndef YEVENT_CALLBACKS_H
#define YEVENT_CALLBACKS_H

#include <functional>

typedef std::function<void()> EventCallback;

typedef std::function<void()> TimerCallback;

typedef std::function<void()> Functor;

#endif
