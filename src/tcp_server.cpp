#include "tcp_server.h"
#include "acceptor.h"
#include <memory>

TcpServer::TcpServer(uint16_t port) :
    _onAcception{std::bind(&TcpServer::onAcception, this)},
    _acceptor{std::make_unique<Acceptor>(port)}
{
}

void TcpServer::listen()
{
    _acceptor->listen();
}

bool TcpServer::isListening() const
{
    return _acceptor->isListening();
}

void TcpServer::onAcception()
{
    if (_onAcception)
    {
        _onAcception();
    }
}