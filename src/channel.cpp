#include "channel.h"
#include "event_loop.h"
#include "log.h"
#include "thread.h"
#include <sys/epoll.h>
#include <sys/socket.h>

Channel::Channel(int fd) :
    _listenEvent{},
    _eventGenerated{},
    _fd{fd},
    _onRead{},
    _onWrite{},
    _onClose{},
    _onError{},
    _index{-1},
    _eventLoop{nullptr}
{
}

Channel::Channel(Channel&& right) :
    _listenEvent{std::move(right._listenEvent)},
    _fd{std::move(right._fd)},
    _onRead{std::move(right._onRead)},
    _onWrite{std::move(right._onWrite)},
    _onClose{std::move(right._onClose)},
    _onError{std::move(right._onError)},
    _index{-1},
    _eventLoop{nullptr}
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
    LOG_ASSERT(_eventGenerated != 0);
    if ((_eventGenerated & EPOLLHUP) && !(_eventGenerated & EPOLLIN))
    {
        if (_onClose)
        {
            _onClose();
        }
    }

    if (_eventGenerated & EPOLLERR)
    {
        if (_onError)
        {
            int       option;
            socklen_t length = sizeof(option);
            bool      error =
                -1 == getsockopt(_fd, SOL_SOCKET, SO_ERROR, &option, &length);
            _onError(error ? errno : option);
        }
    }

    if (_eventGenerated & (EPOLLIN | EPOLLRDHUP | EPOLLPRI))
    {  // ready to read
        if (_onRead)
        {
            _onRead();
        }
    }
    if (_eventGenerated & EPOLLOUT)
    {  // ready to write
        if (_onWrite)
        {
            _onWrite();
        }
    }
    _eventGenerated = 0;
}

void Channel::update()
{
    if (isInEpoll())
    {
        if (_listenEvent == 0)
        {  // listen none event, just remove channel
            _eventLoop->removeChannel(this);
        }
        else
        {
            _eventLoop->modifyChannel(this);
        }
    }
    else
    {
        if (_listenEvent != 0)
        {
            _eventLoop->addChannel(this);
        }
    }
}

void Channel::enableRead()
{
    _listenEvent |= EPOLLIN | EPOLLPRI;
    update();
}

void Channel::disableRead()
{
    _listenEvent &= ~(EPOLLIN | EPOLLPRI);
    update();
}
void Channel::enableWrite()
{
    _listenEvent |= EPOLLOUT;
    update();
}
void Channel::disableWrite()
{
    _listenEvent &= ~EPOLLOUT;
    update();
}

void Channel::enableReadAndWrite()
{
    _listenEvent |= EPOLLIN | EPOLLPRI | EPOLLOUT;
    update();
}

void Channel::disableReadAndWrite()
{
    _listenEvent = 0;
    update();
}

bool Channel::isEnableRead() const
{
    return _listenEvent & (EPOLLIN | EPOLLPRI);
}
bool Channel::isEnbleWrite() const
{
    return _listenEvent & EPOLLOUT;
}

void Channel::onEventLoopChange()
{
    LOG_ASSERT(_eventLoop != &CurrentThread::getEventLoop());
    if (_eventLoop != nullptr)
    {
        _eventLoop->removeChannel(this);
    }
    _eventLoop = &CurrentThread::getEventLoop();
    _eventLoop->addChannel(this);
}

void Channel::setEventLoop(EventLoop* eventLoop)
{
    LOG_ASSERT(_eventLoop == nullptr);
    LOG_ASSERT(eventLoop != nullptr);
    _eventLoop = eventLoop;
}