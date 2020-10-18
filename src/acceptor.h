#pragma once
#include "channel.h"
#include "inet_address.h"
#include "socket.h"
#include <functional>

class Acceptor
{
public:
    Acceptor(const InetAddress& listenAddress);
    ~Acceptor();

    void setOnAcception(std::function<void(Socket)> onNewConnection)
    {
        _onAcception = std::move(onNewConnection);
    }

    void listen();

    constexpr bool isListening() const
    {
        return _isListening;
    }

private:
    void onAcceptable();

private:
    std::function<void(Socket)> _onAcception;
    bool                        _isListening;
    Socket                      _socket;
    Channel                     _channel;
    int                         _reserveFd;
};
