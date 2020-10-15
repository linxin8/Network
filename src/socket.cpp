#include "socket.h"
#include "log.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

Socket::Socket(bool isIp4, bool isBlock, bool isTcp)
{
    int domain = isIp4 ? AF_INET : AF_INET6;
    int type   = isTcp ? SOCK_STREAM : SOCK_DGRAM;
    if (!isBlock)
    {
        type |= SOCK_NONBLOCK;
    }
    _fd = socket(domain, type, NULL);
    if (_fd == -1)
    {
        LOG_ERROR() << std::strerror(errno);
    }
}

Socket::~Socket()
{
    close(_fd);
}

bool Socket::bind(const InetAddress& address)
{
    bool ok = ::bind(_fd,
                     &address.getSocketAddress(),
                     address.getSocketAddressSize()) == 0;
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
    int         fd      = ::accept4(
        _fd, &address.getSocketAddress(), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    bool ok = fd != -1;
    if (!ok)
    {
        LOG_ERROR() << std::strerror(errno);
    }
    return {ok, fd, address};
}

void Socket::setReuseAddress(bool on)
{
    int  value = on;
    bool ok    = -1 == setsockopt(_fd,
                               SOL_SOCKET,
                               SO_REUSEADDR,
                               &value,
                               static_cast<socklen_t>(sizeof(value)));
    if (!ok)
    {
        LOG_ERROR() << std::strerror(errno);
    }
}

void Socket::setReusePort(bool on)
{
    int  value = on;
    bool ok    = -1 == setsockopt(_fd,
                               SOL_SOCKET,
                               SO_REUSEPORT,
                               &value,
                               static_cast<socklen_t>(sizeof(value)));
    if (!ok)
    {
        LOG_ERROR() << std::strerror(errno);
    }
}

void Socket::setKeepAlive(bool on)
{
    int  value = on;
    bool ok    = -1 == setsockopt(_fd,
                               SOL_SOCKET,
                               SO_KEEPALIVE,
                               &value,
                               static_cast<socklen_t>(sizeof(value)));
    if (!ok)
    {
        LOG_ERROR() << std::strerror(errno);
    }
}
