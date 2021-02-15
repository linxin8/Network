#pragma once
#include "acceptor.h"
#include "event_loop_thread_pool.h"
#include <cstdint>
#include <functional>
#include <memory>

class TcpServer
{
public:
    TcpServer(uint16_t port);

    // set number of sub reactor
    // default is 2
    void setSubReactorThreadNumber(int number)
    {
        _threadPool.setThreadNumber(number);
    }

    // multi-thread callback, must check thread safe
    void setOnNewConnection(std::function<void()> onNewConnection)
    {
        _onNewConnection = std::move(onNewConnection);
    }

    // multi-thread callback, must check thread safe
    void setOnReadyToRead(
        std::function<void(std::shared_ptr<TcpConnection>)> onReadyToRead)
    {
        _onReadyToRead = std::move(onReadyToRead);
    }
    void listen();

    bool isListening() const;

private:
    void onNewConnection(std::shared_ptr<TcpConnection> connection);
    void onReadyToRead(std::shared_ptr<TcpConnection>);

private:
    std::function<void()>                                   _onNewConnection;
    std::function<void(std::shared_ptr<TcpConnection>)>     _onReadyToRead;
    Acceptor                                                _acceptor;
    std::unordered_map<int, std::shared_ptr<TcpConnection>> _connectionMap;
    int                                                     _connectionIndex;
    EventLoopThread                                         _acceptorThread;
    EventLoopThreadPool                                     _threadPool;
};
