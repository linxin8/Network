#pragma once
#include "type.h"
#include <atomic>
#include <cassert>
#include <mutex>

class EventLoop : Noncopyable
{
public:
    EventLoop();
    ~EventLoop() = default;

    void start();
    void quit();

private:
    bool _isRunning;
    bool _isQuited;
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