#pragma once
#include "channel.h"
#include "inet_address.h"
#include "socket.h"
#include <functional>

class Acceptor
{
public:
    Acceptor(const InetAddress& listenAddress, bool isReusePort);
    ~Acceptor();

    void setOnNewConnection(
        std::function<void(int, const InetAddress&)> onNewConnection)
    {
        _onNewConnection = std::move(onNewConnection);
    }

    void listen();

    constexpr bool isListening() const
    {
        return _isListening;
    }

private:
    void onRead();

private:
    std::function<void(int, const InetAddress&)> _onNewConnection;
    bool                                         _isListening;
    Socket                                       _socket;
    Channel                                      _channel;
    int                                          _reserveFd;
};
