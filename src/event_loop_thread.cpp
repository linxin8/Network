#include "event_loop_thread.h"
#include "event_loop.h"

EventLoopThread::EventLoopThread() : EventLoopThread{std::function<void()>()} {}
EventLoopThread::EventLoopThread(std::function<void()> initFunc) :
    _thread{std::bind(&EventLoopThread::threadEntry, this)},
    _initFunc{std::move(initFunc)},
    _isEventLoopReady{},
    _eventLoop{nullptr}
{
}
EventLoopThread::EventLoopThread(std::function<void()> initFunc,
                                 std::string           name) :
    _thread{std::bind(&EventLoopThread::threadEntry, this), std::move(name)},
    _initFunc{std::move(initFunc)},
    _isEventLoopReady{},
    _eventLoop{nullptr}
{
}

EventLoopThread::~EventLoopThread()
{
    CurrentThread::getEventLoop().quit();
    _thread.join();
}

void EventLoopThread::start()
{
    LOG_ASSERT(!_thread.isStarted());
    _thread.start();
    while (!_isEventLoopReady)
    {  // spin lock
        CurrentThread::sleep(10);
    }
}

void EventLoopThread::threadEntry()
{
    _eventLoop = std::make_unique<EventLoop>();
    if (_initFunc)
    {
        _initFunc();
    }
    _isEventLoopReady = true;
    _eventLoop->start();
}
