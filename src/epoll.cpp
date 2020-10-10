#include "epoll.h"
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>

EPoll::EPoll() : _fd{epoll_create1(EPOLL_CLOEXEC)}
{
    assert(_fd >= 0);
}

EPoll::~EPoll()
{
    close(_fd);
}

void EPoll::pool(int msecond, std::vector<Channel*> channel)
{
    auto number = ::epoll_wait(_fd, _eventBuffer.data(), static_cast<int>(_eventBuffer.size()), msecond);
    //
}
