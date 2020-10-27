#pragma once
#include "event_loop.h"
#include "thread.h"
#include "type.h"

class EventLoopThread : Noncopyable
{
public:
    EventLoopThread();
    EventLoopThread(std::function<void()> initFunc);
    EventLoopThread(std::function<void()> initFunc, std::string name);
    ~EventLoopThread();

    void startLoop();
    // set function that called once thread is started
    void setInitFunc(std::function<void()> initFunc)
    {
        _initFunc = std::move(initFunc);
    }

    void exec(std::function<void()> function)
    {
        _eventLoop->exec(std::move(function));
    }

private:
    void threadEntry();

private:
    Thread                     _thread;
    std::function<void()>      _initFunc;
    std::atomic_bool           _isEventLoopReady;
    std::unique_ptr<EventLoop> _eventLoop;
};