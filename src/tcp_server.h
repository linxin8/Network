#pragma once
#include <cstdint>
#include <functional>
#include <memory>

class Acceptor;

class TcpServer
{
public:
    TcpServer(uint16_t port);

    void setOnAcception(std::function<void()> callback)
    {
        _onAcception = std::move(callback);
    }

    void listen();

    bool isListening() const;

private:
    void onAcception();

private:
    std::function<void()>     _onAcception;
    std::unique_ptr<Acceptor> _acceptor;
};
