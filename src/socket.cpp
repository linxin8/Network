#include "socket.h"
#include "log.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

bool Socket::bind(const InetAddress& address)
{
    bool ok = ::bind(_fd, &address.getSocketAddress(), address.getSocketAddressSize()) == 0;
    if (!ok)
    {
        LOG_ERROR() << std::strerror(errno);
    }
    return ok;
}

bool Socket::listen()
{
    constexpr int maxInQueue = 4096;
    bool          ok         = ::listen(_fd, maxInQueue) == 0;
    if (!ok)
    {
        LOG_ERROR() << std::strerror(errno);
    }
    return ok;
}

std::tuple<bool, int, InetAddress> Socket::accept()
{
    InetAddress address = InetAddress::getInvalidAddress();
    socklen_t   len     = 0;  // never used;
    int         fd      = ::accept4(_fd, &address.getSocketAddress(), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    bool        ok      = fd != -1;
    if (!ok)
    {
        LOG_ERROR() << std::strerror(errno);
    }
    return {ok, fd, address};
}