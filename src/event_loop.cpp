#include "event_loop.h"
#include "channel.h"
#include "epoll.h"
#include "log.h"
#include <cassert>

EventLoop::EventLoop() :
    _isRunning{},
    _isQuited{},
    _channelVector{},
    _epoll{std::make_unique<EPoll>()}

{
    CurrentThread::setEventLoop(const_cast<EventLoop&>(*this));
}

void EventLoop::start()
{
    assert(!_isRunning);
    while (!_isQuited)
    {
        constexpr int mtime = 10000;
        _epoll->pool(mtime, _channelVector);
        for (auto& channel : _channelVector)
        {
            channel->handleEvent();
        }
    }
}
void EventLoop::quit()
{
    _isQuited = true;
}

void EventLoop::loop() {}

namespace CurrentThread
{
    thread_local EventLoop* _eventLoop = nullptr;
}  // namespace CurrentThread