#pragma once
#include "type.h"
#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <vector>

class EPoll;

class Channel;

class EventLoop : Noncopyable
{
public:
    EventLoop();
    ~EventLoop() = default;

    void start();
    void quit();

    void EventLoop::modifyChannel(Channel* channel)
    {
        assert(channel->isInEpoll());
        _epoll->modify(channel);
    }

    void EventLoop::addChannel(Channel* channel)
    {
        assert(!channel->isInEpoll());
        _epoll->add(channel);
    }

    void EventLoop::removeChannel(Channel* channel)
    {
        assert(channel->isInEpoll());
        _epoll->remove(channel);
    }

private:
    void loop();

private:
    bool                   _isRunning;
    bool                   _isQuited;
    std::vector<Channel*>  _channelVector;
    std::unique_ptr<EPoll> _epoll;
};

namespace CurrentThread
{
    extern thread_local EventLoop* _eventLoop;
    inline EventLoop&              getEventLoop()
    {
        assert(_eventLoop != nullptr);
        return *_eventLoop;
    }
    // only allow set once
    inline void setEventLoop(EventLoop& loop)
    {
        assert(_eventLoop == nullptr);
        _eventLoop = &loop;
    }
};  // namespace CurrentThread