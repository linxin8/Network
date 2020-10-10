#include "event_loop.h"
#include "log.h"
#include <cassert>

EventLoop::EventLoop() : _isRunning{}, _isQuited{}
{
    CurrentThread::setEventLoop(const_cast<EventLoop&>(*this));
}

void EventLoop::start()
{
    assert(!_isRunning);
    while (!_isQuited) {}
}

namespace CurrentThread
{
    thread_local EventLoop* _eventLoop = nullptr;
}  // namespace CurrentThread