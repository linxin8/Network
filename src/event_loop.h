#pragma once
#include "channel.h"
#include "epoll.h"
#include "log.h"
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
        LOG_ASSERT(channel->isInEpoll());
        _epoll->modify(channel);
    }

    void addChannel(Channel* channel)
    {
        LOG_ASSERT(!channel->isInEpoll());
        _epoll->add(channel);
    }

    void removeChannel(Channel* channel)
    {
        LOG_ASSERT(channel->isInEpoll());
        _epoll->remove(channel);
    }

    // thread safe
    void exec(std::function<void()> function)
    {
        {
            std::lock_guard<std::mutex> guard{_mutex};
            _functionToDo.emplace_back(std::move(function));
        }
        bool error = -1 == ::write(_wakeUpPipe[1], "", 1);  // wake up
        if (error)
        {
            LOG_ERROR() << std::strerror(errno);
        }
    }

private:
    void loop();

private:
    bool                               _isRunning;
    bool                               _isQuited;
    std::vector<Channel*>              _channelVector;
    std::unique_ptr<EPoll>             _epoll;
    std::vector<std::function<void()>> _functionToDo;
    std::mutex                         _mutex;
    std::array<int, 2>                 _wakeUpPipe;
    Channel                            _wakeUpChannel;
};

namespace CurrentThread
{
    extern thread_local EventLoop* _eventLoop;
    inline EventLoop&              getEventLoop()
    {
        LOG_ASSERT(_eventLoop != nullptr);
        return *_eventLoop;
    }
    // only allow set once
    inline void setEventLoop(EventLoop& loop)
    {
        LOG_ASSERT(_eventLoop == nullptr);
        _eventLoop = &loop;
    }
};  // namespace CurrentThread