#pragma once
#include "event_loop_thread.h"
#include "type.h"

class EventLoopThreadPool : Noncopyable
{
public:
    explicit EventLoopThreadPool(int threadNumber = 1);
    void setThreadNumber(int threadNumber)
    {
        LOG_ASSERT(threadNumber >= 1);
        LOG_ASSERT(!_isStarted);
        _threadNumber = threadNumber;
    }
    // start all thread in pool
    void                             start();
    std::shared_ptr<EventLoopThread> getNextLoopThread();

private:
    int                                           _threadNumber;
    std::vector<std::shared_ptr<EventLoopThread>> _pool;
    int                                           _nextPoolIndex;
    bool                                          _isStarted;
};
