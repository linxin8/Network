#pragma once

#include "inet_address.h"

class Socket
{
public:
    // ip4 or ip6
    // block or nonblock
    // tcp or udp
    Socket(bool isIp4, bool isBlock, bool isTcp);
    ~Socket();

    int getFd() const
    {
        return _fd;
    }
    // skip TIME_AWAIT protect step, default is false
    void setReuseAddress(bool on);
    //  muti-socket can listen on the same port
    // (except one already in TIME_AWAIT step), default is false
    void setReusePort(bool on);
    void setKeepAlive(bool on);

    bool                               listen();
    bool                               bind(const InetAddress& address);
    std::tuple<bool, int, InetAddress> accept();

private:
    int _fd;
};
