#pragma once
#include "thread.h"
#include "type.h"

class EventLoopThread : Noncopyable
{
public:
    EventLoopThread();
    EventLoopThread(std::function<void()> initFunc);
    EventLoopThread(std::function<void()> initFunc, std::string name);
    ~EventLoopThread();

    void start();

private:
    void threadEntry();

private:
    Thread                _thread;
    std::function<void()> _initFunc;
    std::atomic_bool      _isEventLoopReady;
};