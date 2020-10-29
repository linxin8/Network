#include "event_loop_thread_pool.h"

EventLoopThreadPool::EventLoopThreadPool(int number) :
    _threadNumber{number}, _pool{}, _nextPoolIndex{}, _isStarted{}
{
}

void EventLoopThreadPool::start()
{
    LOG_ASSERT(!_isStarted);
    _isStarted = true;
    for (int i = 0; i < _threadNumber; i++)
    {
        _pool.emplace_back(std::make_shared<EventLoopThread>());
    }
    for (int i = 1; i <= _threadNumber; i++)
    {
        auto& thread = _pool[i - 1];
        thread->start();
        thread->exec([i] {
            CurrentThread::setName(std::string("poolthread") +
                                   std::to_string(i));
        });
    }
}

std::shared_ptr<EventLoopThread> EventLoopThreadPool::getNextLoopThread()
{
    return _pool[_nextPoolIndex++ % _pool.size()];
}
