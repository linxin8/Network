#pragma once
#include "channel.h"
#include "socket.h"
#include "tcp_buffer.h"

class TcpConnection
{
public:
    TcpConnection(Socket socket);

public:
    void send(const void* data, size_t size);
    void recv();

private:
    void onRead();
    void onWrite();
    void onError();
    void onClose();

private:
    std::string           _name;
    Socket                _socket;
    Channel               _channel;
    TcpBuffer             _readBuffer;
    TcpBuffer             _writeBuffer;
    std::function<void()> _onRead;
    std::function<void()> _onWrite;
    std::function<void()> _onError;
    std::function<void()> _onClose;
};