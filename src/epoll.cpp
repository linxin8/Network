#include "epoll.h"
#include "channel.h"
#include "log.h"
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>

EPoll::EPoll() : _fd{epoll_create1(EPOLL_CLOEXEC)}, _eventBuffer{8}
{
    LOG_ASSERT(_fd >= 0);
}

EPoll::~EPoll()
{
    close(_fd);
}

void EPoll::pool(int msecond, std::vector<Channel*>& channel)
{
    auto number = ::epoll_wait(_fd,
                               _eventBuffer.data(),
                               static_cast<int>(_eventBuffer.size()),
                               msecond);
    if (number == -1)
    {
        LOG_ERROR() << std::strerror(errno);
    }
    else
    {
        for (int i = 0; i < number; i++)
        {
            auto& event = _eventBuffer[i];
            auto  ch    = static_cast<Channel*>(event.data.ptr);
            ch->setEventGenerated(event.events);
            channel.push_back(ch);
        }
        if (number == _eventBuffer.size())
        {
            _eventBuffer.resize(static_cast<size_t>(number * 1.5));
        }
    }
}

void EPoll::add(Channel* channel)
{
    LOG_ASSERT(channel->getIndex() == -1);
    LOG_TRACE() << "channel" << channel << "fd" << channel->getFd();
    channel->setIndex(static_cast<int>(_channelVector.size()));
    _channelVector.push_back(channel);
    control(EPOLL_CTL_ADD, channel);
}

void EPoll::modify(Channel* channel)
{
    LOG_ASSERT(_channelVector.at(channel->getIndex()) == channel);
    LOG_TRACE() << "channel" << channel << "fd" << channel->getFd();
    control(EPOLL_CTL_MOD, channel);
}

void EPoll::remove(Channel* channel)
{
    LOG_ASSERT(_channelVector.at(channel->getIndex()) == channel);
    LOG_TRACE() << "channel" << channel << "fd" << channel->getFd();
    control(EPOLL_CTL_DEL, channel);
    auto index = channel->getIndex();
    _channelVector.back()->setIndex(index);
    std::swap(_channelVector.at(index), _channelVector.back());
    _channelVector.pop_back();
    channel->setIndex(-1);
}

void EPoll::control(int operation, Channel* channel)
{
    epoll_event event;
    event.data.ptr = static_cast<void*>(channel);
    event.events   = channel->getListenEvents();
    bool error     = -1 == epoll_ctl(_fd, operation, channel->getFd(), &event);
    if (error)
    {
        LOG_ERROR() << std::strerror(errno);
    }
}
