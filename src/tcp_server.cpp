#include "tcp_server.h"
#include <atomic>
#include <memory>

TcpServer::TcpServer(uint16_t port) :
    _onNewConnection{},
    _onReadyToRead{},
    _acceptor{port},
    _connectionMap{},
    _connectionIndex{0},
    _acceptorThread{},
    _threadPool{},
    _eventLoop{&CurrentThread::getEventLoop()}
{
    _acceptor.setOnAcception([this](std::shared_ptr<TcpConnection> connection) {
        (this->*&TcpServer::onNewConnection)(std::move(connection));
    });
    _acceptorThread.start();
    _acceptorThread.exec([] { CurrentThread::setName("acceptor"); });
    _threadPool.setThreadNumber(2);
    _acceptor.setEventLoop(_acceptorThread.getEventLoop());
}

void TcpServer::listen()
{
    LOG_ASSERT(!isListening());
    _threadPool.start();
    _acceptorThread.exec([this]() { _acceptor.listen(); });
}

bool TcpServer::isListening() const
{
    return _acceptor.isListening();
}

void TcpServer::onNewConnection(std::shared_ptr<TcpConnection> connection)
{
    _acceptorThread.checkInOwnLoop();
    std::weak_ptr<TcpConnection> weakConnection = connection;
    connection->setOnReadyToRead(
        [weakConnection, this](std::shared_ptr<TcpConnection>) {
            if (auto con = weakConnection.lock())
            {
                onReadyToRead(std::move(con));
            }
        });
    auto thread = _threadPool.getNextLoopThread();
    thread->exec(
        [connection, thread] { thread->insertConnection(connection); });
    if (_onNewConnection)
    {
        _onNewConnection();
    }
}

void TcpServer::onReadyToRead(std::shared_ptr<TcpConnection> connection)
{
    if (_onReadyToRead)
    {
        _onReadyToRead(connection);
    }
}