#pragma once
#include "channel.h"
#include "socket.h"
#include "tcp_buffer.h"
#include "type.h"

class TcpConnection : Noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
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
    void setOnReadyToRead(
        std::function<void(std::shared_ptr<TcpConnection>)> onReadyToRead)
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

    void enableRead()
    {
        _channel.enableRead();
    }

    void enableWrite()
    {
        _channel.enableWrite();
    }

    void enableReadAndWrite()
    {
        _channel.enableReadAndWrite();
    }

    void disableRead()
    {
        _channel.disableRead();
    }

    void disableWrite()
    {
        _channel.isEnbleWrite();
    }

    void disableReadAndWrite()
    {
        _channel.disableReadAndWrite();
    }

    void setEventLoop(EventLoop* eventLoop)
    {
        _channel.setEventLoop(eventLoop);
    }

    void close();

private:
    void onRead();
    void onWrite();
    void onError(int errorNo);
    void onClose();

private:
    std::string                                         _name;
    Socket                                              _socket;
    Channel                                             _channel;
    TcpBuffer                                           _sendBuffer;
    TcpBuffer                                           _recvBuffer;
    std::function<void(std::shared_ptr<TcpConnection>)> _onReadyToRead;
    std::function<void(size_t)>                         _onSent;
    std::function<void(int errorNo)>                    _onError;
    std::function<void()>                               _onClose;
};