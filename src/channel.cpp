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

Channel::~Channel() {}

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
bool Channel::isEnableRead()
{
    return _events & (EPOLLIN | EPOLLPRI);
}
bool Channel::isEnbleWrite()
{
    return _events & EPOLLOUT;
}