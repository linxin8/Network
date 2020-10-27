#include "tcp_server.h"
#include <atomic>
#include <memory>

TcpServer::TcpServer(uint16_t port) :
    _onNewConnection{},
    _acceptor{port},
    _connectionMap{},
    _connectionIndex{0},
    _acceptorThread{}
{
    _acceptor.setOnAcception([this](std::unique_ptr<TcpConnection> connection) {
        (this->*&TcpServer::onNewConnection)(std::move(connection));
    });
    _acceptorThread.startLoop();
}

void TcpServer::listen()
{
    LOG_ASSERT(!isListening());
    _acceptorThread.exec([this]() { _acceptor.listen(); });
}

void TcpServer::close(int number)
{
    LOG_ASSERT(_connectionMap.count(number) > 0);
    _connectionMap.erase(number);
}

bool TcpServer::isListening() const
{
    return _acceptor.isListening();
}

void TcpServer::onNewConnection(std::unique_ptr<TcpConnection> connection)
{
    if (_onNewConnection)
    {
        _onNewConnection();
    }
    int index = _connectionIndex;
    connection->setOnReadyToRead(
        [index, this](size_t) { onReadyToRead(index); });
    connection->getSocket().getErrorNo();
    connection->setOnError(
        [](int errorNo) { LOG_ERROR() << std::strerror(errorNo); });
    _connectionMap.emplace(_connectionIndex++, std::move(connection));
}

void TcpServer::onReadyToRead(int id)
{
    if (_onReadyToRead)
    {
        _onReadyToRead(id);
    }
}