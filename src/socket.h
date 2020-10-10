#pragma once

#include "inet_address.h"

class Socket
{
private:
    /* data */
public:
    explicit Socket(int fd) : _fd{fd} {}
    ~Socket();

    int getFd() const
    {
        return _fd;
    }

    bool                               listen();
    bool                               bind(const InetAddress& address);
    std::tuple<bool, int, InetAddress> accept();

private:
    const int _fd;
};
