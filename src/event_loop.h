#pragma once
#include "channel.h"
#include "epoll.h"
#include "type.h"
#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <vector>

class EventLoop : Noncopyable
{
public:
    EventLoop();
    ~EventLoop() = default;

    // infinite loop, never return until quit is called
    void start();
    void quit();

    void modifyChannel(Channel* channel)
    {
        assert(channel->isInEpoll());
        _epoll->modify(channel);
    }

    void addChannel(Channel* channel)
    {
        assert(!channel->isInEpoll());
        _epoll->add(channel);
    }

    void removeChannel(Channel* channel)
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