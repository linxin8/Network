#pragma once

#include "inet_address.h"
#include "type.h"

class Socket : Noncopyable
{
public:
    // create socket
    // ip4 or ip6
    // block or nonblock
    // tcp or udp
    Socket(bool isIp4, bool isBlock, bool isTcp);
    Socket(Socket&& socket) : _fd{socket._fd}
    {
        socket._fd = -1;
    }

    int getErrorNo() const;

    void close();
    // create socket with fd
    // take ownship
    Socket(int socketFd);
    ~Socket();

    InetAddress getLocalAddress() const;
    InetAddress getPeerAddress() const;

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

    bool                    listen();
    bool                    bind(const InetAddress& address);
    std::pair<bool, Socket> accept();

private:
    int _fd;
};
