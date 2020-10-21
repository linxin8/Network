#include "channel.h"
#include "event_loop.h"
#include "log.h"
#include "thread.h"
#include <sys/epoll.h>

Channel::Channel(int fd) :
    _events{},
    _fd{fd},
    _onRead{},
    _onWrite{},
    _onClose{},
    _onError{},
    _index{-1}
{
}

Channel::Channel(Channel&& right) :
    _events{std::move(right._events)},
    _fd{std::move(right._fd)},
    _onRead{std::move(right._onRead)},
    _onWrite{std::move(right._onWrite)},
    _onClose{std::move(right._onClose)},
    _onError{std::move(right._onError)},
    _index{-1}
{
    if (right.isEnableRead() && right.isEnbleWrite())
    {
        right.disableReadAndWrite();
        enableReadAndWrite();
    }
    else if (right.isEnableRead())
    {
        right.disableRead();
        enableRead();
    }
    else if (right.isEnbleWrite())
    {
        right.disableWrite();
        enableWrite();
    }
}

Channel::~Channel()
{
    disableReadAndWrite();
}

void Channel::handleEvent()
{
    if ((_events & EPOLLHUP) && !(_events & EPOLLIN))
    {
        if (_onClose)
        {
            _onClose();
        }
    }

    if (_events & EPOLLERR)
    {
        LOG_ERROR() << "error occurred EPOLLERR";
        if (_onError)
        {
            _onError();
        }
    }

    if (_events & (EPOLLIN | EPOLLRDHUP | EPOLLPRI))
    {  // ready to read
        if (_onRead)
        {
            _onRead();
        }
    }
    if (_events & EPOLLOUT)
    {  // ready to write
        if (_onWrite)
        {
            _onWrite();
        }
    }
}

void Channel::update()
{
    auto& eventLoop = CurrentThread::getEventLoop();
    if (isInEpoll())
    {
        if (_events == 0)
        {  // listen none event, just remove channel
            eventLoop.removeChannel(this);
        }
        else
        {
            eventLoop.modifyChannel(this);
        }
    }
    else
    {
        if (_events != 0)
        {
            eventLoop.addChannel(this);
        }
    }
}

void Channel::enableRead()
{
    _events |= EPOLLIN | EPOLLPRI;
    update();
}

void Channel::disableRead()
{
    _events &= ~(EPOLLIN | EPOLLPRI);
    update();
}
void Channel::enableWrite()
{
    _events |= EPOLLOUT;
    update();
}
void Channel::disableWrite()
{
    _events &= ~EPOLLOUT;
    update();
}

void Channel::enableReadAndWrite()
{
    _events |= EPOLLIN | EPOLLPRI | EPOLLOUT;
    update();
}

void Channel::disableReadAndWrite()
{
    _events = 0;
    update();
}

bool Channel::isEnableRead() const
{
    return _events & (EPOLLIN | EPOLLPRI);
}
bool Channel::isEnbleWrite() const
{
    return _events & EPOLLOUT;
}