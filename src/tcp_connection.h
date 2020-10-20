#pragma once
#include "channel.h"
#include "socket.h"
#include "tcp_buffer.h"

class TcpConnection
{
public:
    TcpConnection(Socket socket);

public:
    // send size-fixed data asynchronously
    void sendAsyn(const void* data, size_t size);

    // received the data with the maximun size
    // return acctually size of data received
    size_t recv(const void* data, size_t maxSize);

    // called when data is sent
    // argument is the acctually size of data sent
    void setOnSent(std::function<void(size_t)> onSent)
    {
        _onSent = std::move(onSent);
    }

    // called when data is ready to read
    // argument is the acctually size of data that can be read
    void setOnReadyToRead(std::function<void(size_t)> onReadyToRead)
    {
        _onReadyToRead = std::move(onReadyToRead);
    }

    void setOnError(std::function<void()> onError)
    {
        _onError = std::move(onError);
    }

    void setOnClose(std::function<void()> onClose)
    {
        _onClose = std::move(onClose);
    }

private:
    void onRead();
    void onWrite();
    void onError();
    void onClose();

private:
    std::string                 _name;
    Socket                      _socket;
    Channel                     _channel;
    std::vector<TcpBuffer>      _sendBuffer;
    TcpBuffer                   _recvBuffer;
    std::function<void(size_t)> _onReadyToRead;
    std::function<void(size_t)> _onSent;
    std::function<void()>       _onError;
    std::function<void()>       _onClose;
};