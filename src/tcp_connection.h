#pragma once
#include "channel.h"
#include "socket.h"
#include "tcp_buffer.h"
#include "type.h"

class TcpConnection : Noncopyable
{
public:
    TcpConnection(Socket socket);

public:
    // send size-fixed data asynchronously
    size_t sendAsyn(const void* data, size_t size);

    // received the data with the maximun size
    // return acctually size of data received
    size_t recv(void* data, size_t maxSize);

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

    void setOnError(std::function<void(int errorNo)> onError)
    {
        _onError = std::move(onError);
    }

    void setOnClose(std::function<void()> onClose)
    {
        _onClose = std::move(onClose);
    }

    const Socket& getSocket() const
    {
        return _socket;
    }

    TcpBuffer& getRecvBuffer()
    {
        return _recvBuffer;
    }

    void close();

private:
    void onRead();
    void onWrite();
    void onError();
    void onClose();

private:
    std::string                      _name;
    Socket                           _socket;
    Channel                          _channel;
    TcpBuffer                        _sendBuffer;
    TcpBuffer                        _recvBuffer;
    std::function<void(size_t)>      _onReadyToRead;
    std::function<void(size_t)>      _onSent;
    std::function<void(int errorNo)> _onError;
    std::function<void()>            _onClose;
};