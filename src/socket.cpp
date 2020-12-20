#include "socket.h"
#include "log.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <systemd/sd-daemon.h>
#include <unistd.h>

Socket::Socket(int fd) : _fd{fd}
{
    // omly check if is socket, not check family, type, listening state
    LOG_ASSERT(sd_is_socket(fd, AF_UNSPEC, 0, -1));
}

Socket::Socket(bool isIp4, bool isBlock, bool isTcp)
{
    int domain = isIp4 ? AF_INET : AF_INET6;
    int type   = isTcp ? SOCK_STREAM : SOCK_DGRAM;
    if (!isBlock)
    {
        type |= SOCK_NONBLOCK;
    }
    _fd = socket(domain, type, 0);
    if (_fd == -1)
    {
        LOG_ERROR() << std::strerror(errno);
    }
}

Socket::~Socket()
{
    close();
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

std::pair<bool, Socket> Socket::accept()
{
    InetAddress address = InetAddress::getInvalidAddress();
    socklen_t   len     = sizeof(address);
    int         fd      = ::accept4(
        _fd, &address.getSocketAddress(), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);

    bool error = fd == -1;

    if (error)
    {
        // if (errno == EAGAIN || errno == EWOULDBLOCK)
        // {
        // The socket is marked nonblocking and no connections are present
        // to be accepted. do nothing.
        // }
        // else
        // {
        LOG_ERROR() << std::strerror(errno);
        // }
    }
    return {!error, Socket{fd}};
}

void Socket::setReuseAddress(bool on)
{
    int  value = on;
    bool ok    = -1 != setsockopt(_fd,
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
    bool error = -1 == setsockopt(_fd,
                                  SOL_SOCKET,
                                  SO_REUSEPORT,
                                  &value,
                                  static_cast<socklen_t>(sizeof(value)));
    if (error)
    {
        LOG_ERROR() << std::strerror(errno);
    }
}

void Socket::setKeepAlive(bool on)
{
    int  value = on;
    bool error = -1 == setsockopt(_fd,
                                  SOL_SOCKET,
                                  SO_KEEPALIVE,
                                  &value,
                                  static_cast<socklen_t>(sizeof(value)));
    if (error)
    {
        LOG_ERROR() << std::strerror(errno);
    }
}

InetAddress Socket::getPeerAddress() const
{
    InetAddress address = InetAddress::getInvalidAddress();
    socklen_t   length  = sizeof(address);
    bool        error =
        -1 == getpeername(_fd,
                          static_cast<sockaddr*>(&address.getSocketAddress()),
                          &length);
    if (error)
    {
        LOG_ERROR() << std::strerror(errno);
        return InetAddress::getInvalidAddress();
    }
    return address;
}

InetAddress Socket::getLocalAddress() const
{
    InetAddress address = InetAddress::getInvalidAddress();
    socklen_t   length  = sizeof(address);
    bool        error =
        -1 == getsockname(_fd,
                          static_cast<sockaddr*>(&address.getSocketAddress()),
                          &length);
    if (error)
    {
        LOG_ERROR() << std::strerror(errno);
        return InetAddress::getInvalidAddress();
    }
    return address;
}

void Socket::close()
{
    if (_fd != -1)
    {
        bool error = -1 == ::close(_fd);
        if (error)
        {
            LOG_ERROR() << std::strerror(errno);
        }
        _fd = -1;
    }
}

int Socket::getErrorNo() const
{
    int       option;
    socklen_t length = sizeof(option);
    bool error = -1 == getsockopt(_fd, SOL_SOCKET, SO_ERROR, &option, &length);
    return error ? errno : option;
}

ssize_t Socket::sendNonblocking(const void* data, size_t maxSize)
{
    LOG_ASSERT(data != nullptr);
    ssize_t size = ::send(_fd, data, maxSize, MSG_DONTWAIT);
    if (size == -1)
    {
        LOG_ERROR() << std::strerror(errno);
    }
    return size;
}

ssize_t Socket::recvNonblocking(void* data, size_t maxSize)
{
    LOG_ASSERT(data != nullptr);
    ssize_t size = ::recv(_fd, data, maxSize, MSG_DONTWAIT);
    if (size == -1)
    {
        LOG_ERROR() << std::strerror(errno) << "fd" << _fd;
    }
    return size;
}

bool Socket::connect(InetAddress address)
{
    int no = ::connect(
        _fd, &address.getSocketAddress(), address.getSocketAddressSize());
    if (no != 0)
    {
        int err = errno;
        if (err != EINPROGRESS)
        {  // EINPROGRESS is not error for nonblock socket
            LOG_ERROR() << std::strerror(err);
            return false;
        }
    }
    return true;
}