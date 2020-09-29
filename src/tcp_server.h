#pragma once
#include <functional>

class TcpServer
{
public:
    using CallBack = std::function<void()>;

public:
    TcpServer();

    void setOnConection(const CallBack& callback)
    {
        _onConnection = callback;
    }
    void setonMessageReceived(const CallBack& callback)
    {
        _onMessageReceived = callback;
    }
    void setonMessageSend(const CallBack& callback)
    {
        _onMessageSend = callback;
    }

private:
    CallBack _onConnection;
    CallBack _onMessageReceived;
    CallBack _onMessageSend;
};
