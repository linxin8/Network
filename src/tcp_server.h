#pragma once
#include "acceptor.h"
#include "event_loop_thread.h"
#include <cstdint>
#include <functional>
#include <memory>

class TcpServer
{
public:
    TcpServer(uint16_t port);

    void setOnNewConnection(std::function<void()> onNewConnection)
    {
        _onNewConnection = std::move(onNewConnection);
    }
    void setOnReadyToRead(std::function<void(int)> onReadyToRead)
    {
        _onReadyToRead = std::move(onReadyToRead);
    }
    void listen();

    bool isListening() const;

    void close(int number);

private:
    void onNewConnection(std::unique_ptr<TcpConnection> connection);
    void onReadyToRead(int id);

private:
    std::function<void()>                                   _onNewConnection;
    std::function<void(int)>                                _onReadyToRead;
    Acceptor                                                _acceptor;
    std::unordered_map<int, std::unique_ptr<TcpConnection>> _connectionMap;
    int                                                     _connectionIndex;
    EventLoopThread                                         _acceptorThread;
};

class TcpConnectionController
{
public:
    TcpConnectionController(int id, TcpServer* server) :
        _id{id}, _server{server}
    {
    }
    void send(std::vector<char> data) {}
    void read();

private:
    int        _id;
    TcpServer* _server;
};
