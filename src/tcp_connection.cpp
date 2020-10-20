#include "tcp_connection.h"
#include "log.h"

TcpConnection::TcpConnection(Socket socket) :
    _name{},
    _socket{std::move(socket)},
    _channel{socket.getFd()},
    _sendBuffer{},
    _recvBuffer{},
    _onReadyToRead{},
    _onSent{},
    _onError{},
    _onClose{}
{
    _channel.setOnClose(std::bind(&TcpConnection::onClose, this));
    _channel.setOnError(std::bind(&TcpConnection::onError, this));
    _channel.setOnRead(std::bind(&TcpConnection::onRead, this));
    _channel.setOnWrite(std::bind(&TcpConnection::onWrite, this));
}

void TcpConnection::sendAsyn(const void* data, size_t size)
{
    _sendBuffer.push_back({});
    _sendBuffer.back().append(data, size);
}

void TcpConnection::onRead()
{
    constexpr int bufferSize = 65535;
    char          buffer[bufferSize];
    ssize_t       size = _socket.recvNonblocking(buffer, bufferSize);
    if (size == -1)
    {
        onError();
    }
    else
    {
        _recvBuffer.append(buffer, size);
    }
}