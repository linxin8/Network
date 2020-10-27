#include "event_loop.h"
#include "channel.h"
#include "epoll.h"
#include "fcntl.h"
#include "log.h"
#include <sys/eventfd.h>

EventLoop::EventLoop() :
    _isRunning{},
    _isQuited{},
    _channelVector{},
    _epoll{std::make_unique<EPoll>()},
    _wakeUpPipe{[] {
        std::array<int, 2> pip;
        int                temp[2];
        LOG_ASSERT(-1 != pipe2(temp, O_NONBLOCK));
        pip[0] = temp[0];
        pip[1] = temp[1];
        return std::move(pip);
    }()},
    _wakeUpChannel{_wakeUpPipe[0]}
{
    CurrentThread::setEventLoop(const_cast<EventLoop&>(*this));
    _epoll->add(&_wakeUpChannel);
    _wakeUpChannel.setOnRead([fd = _wakeUpPipe[0]] {
        uint64_t value;
        bool     error = -1 == ::read(fd, &value, sizeof(value));
        if (error)
        {
            LOG_ERROR() << std::strerror(errno);
        }
    });
    _wakeUpChannel.enableRead();
}

void EventLoop::start()
{
    LOG_ASSERT(!_isRunning);
    std::vector<std::function<void()>> functionTodo;
    while (!_isQuited)
    {
        constexpr int mtime = 10000;
        _epoll->pool(mtime, _channelVector);
        for (auto& channel : _channelVector)
        {
            channel->handleEvent();
        }
        _channelVector.clear();
        {
            std::lock_guard<std::mutex> guard{_mutex};
            functionTodo.swap(_functionToDo);
        }
        for (auto& f : functionTodo)
        {
            f();
        }
        functionTodo.clear();
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