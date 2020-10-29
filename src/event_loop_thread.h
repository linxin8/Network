#pragma once
#include "event_loop.h"
#include "tcp_connection.h"
#include "thread.h"
#include "type.h"

class EventLoopThread : Noncopyable
{
public:
    EventLoopThread();
    EventLoopThread(std::function<void()> initFunc);
    EventLoopThread(std::function<void()> initFunc, std::string name);
    ~EventLoopThread();

    void start();
    // set function that called once thread is started
    void setInitFunc(std::function<void()> initFunc)
    {
        _initFunc = std::move(initFunc);
    }

    void exec(std::function<void()> function)
    {
        _eventLoop->exec(std::move(function));
    }

    void insertConnection(std::shared_ptr<TcpConnection> connection)
    {
        checkInOwnLoop();
        int id = ++_connectionId;
        LOG_ASSERT(_connectionMap.count(id) == 0);
        _connectionMap.emplace(id, connection);
        connection->setOnClose([id, &map = _connectionMap, this] {
            LOG_ASSERT(map.count(id) == 1);
            exec([id, &map] { map.erase(id); });
        });
        connection->setOnError(
            [](int errorNo) { LOG_ERROR() << std::strerror(errorNo); });
        connection->setEventLoop(_eventLoop.get());
        connection->enableRead();
    }

    EventLoop* getEventLoop()
    {
        return _eventLoop.get();
    }
    void checkInOwnLoop() const
    {
        LOG_ASSERT(&CurrentThread::getEventLoop() == _eventLoop.get());
    }

private:
    void threadEntry();

private:
    Thread                                                  _thread;
    std::function<void()>                                   _initFunc;
    std::atomic_bool                                        _isEventLoopReady;
    std::unique_ptr<EventLoop>                              _eventLoop;
    std::unordered_map<int, std::shared_ptr<TcpConnection>> _connectionMap;
    int                                                     _connectionId;
};