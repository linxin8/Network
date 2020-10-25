#pragma once
#include "channel.h"
#include "inet_address.h"
#include "socket.h"
#include "tcp_connection.h"
#include <functional>

class Acceptor
{
public:
    Acceptor(const InetAddress& listenAddress);
    ~Acceptor();

    void setOnAcception(
        std::function<void(std::unique_ptr<TcpConnection>)> onNewConnection)
    {
        _onAcception = std::move(onNewConnection);
    }

    void listen();

    constexpr bool isListening() const
    {
        return _isListening;
    }

private:
    void onAcception();

private:
    std::function<void(std::unique_ptr<TcpConnection>)> _onAcception;
    bool                                                _isListening;
    Socket                                              _socket;
    Channel                                             _channel;
    int                                                 _reserveFd;
};
