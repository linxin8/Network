#include "tcp_connection.h"
#include "log.h"

TcpConnection::TcpConnection(Socket socket) :
    _name{},
    _socket{std::move(socket)},
    _channel{_socket.getFd()},
    _sendBuffer{},
    _recvBuffer{},
    _onReadyToRead{},
    _onSent{},
    _onError{},
    _onClose{}
{
    _channel.setOnClose(std::bind(&TcpConnection::onClose, this));
    _channel.setOnError(
        std::bind(&TcpConnection::onError, this, std::placeholders::_1));
    _channel.setOnRead(std::bind(&TcpConnection::onRead, this));
    _channel.setOnWrite(std::bind(&TcpConnection::onWrite, this));
}

void TcpConnection::sendAsyn(const void* data, size_t size)
{
    if (size != 0)
    {
        if (!_channel.isEnbleWrite())
        {
            _channel.enableWrite();
        }
        _sendBuffer.append(data, size);
    }
}

void TcpConnection::sendAsyn(const std::string& data)
{
    if (data.size())
    {
        if (!_channel.isEnbleWrite())
        {
            _channel.enableWrite();
        }
        _sendBuffer.append(data);
    }
}

std::string TcpConnection::recvAll()
{
    char        buffer[1024];
    std::string data;
    while (true)
    {
        auto n = recv(buffer, 1024);
        if (n == 0)
        {
            break;
        }
        data.append(buffer, n);
    }
    return std::move(data);
}

size_t TcpConnection::recv(void* data, size_t maxSize)
{
    size_t copyedSize = 0;
    void*  dest       = data;
    while (_recvBuffer.getContinousSize() != 0 && copyedSize < maxSize)
    {
        size_t size =
            std::min(maxSize - copyedSize, _recvBuffer.getContinousSize());
        dest = memcpy(dest, _recvBuffer.getFirstAddress(), size);
        _recvBuffer.popN(size);
        copyedSize += size;
    }
    return copyedSize;
}

size_t TcpConnection::recv(std::string& data)
{
    size_t copyedSize = 0;
    while (_recvBuffer.getContinousSize() != 0)
    {
        size_t size = _recvBuffer.getContinousSize();
        data.append(_recvBuffer.getFirstAddress(), size);
        _recvBuffer.popN(size);
        copyedSize += size;
    }
    return copyedSize;
}

void TcpConnection::onRead()
{
    char    buffer[1024];
    ssize_t size = _socket.recvNonblocking(buffer, 1024);
    if (size == -1)
    {
        onError(errno);
    }
    else if (size == 0)
    {
        // orderly shutdown of the connection
        onClose();
    }
    else if (size > 0)
    {
        _recvBuffer.append(buffer, size);
        if (_onReadyToRead)
        {
            _onReadyToRead(shared_from_this());
        }
    }
}

void TcpConnection::onWrite()
{
    size_t size = _socket.sendNonblocking(_sendBuffer.getFirstAddress(),
                                          _sendBuffer.getContinousSize());
    if (size == -1)
    {
        onError(errno);
    }
    else if (size > 0)
    {
        _sendBuffer.popN(size);
        if (_sendBuffer.getSize() == 0)
        {
            // unregister channel writing event
            _channel.disableWrite();
        }
        if (_onSent)
        {
            _onSent(size);
        }
    }
}

void TcpConnection::onError(int errorNo)
{
    LOG_ERROR() << std::strerror(errorNo);
}

void TcpConnection::onClose()
{
    if (_onClose)
    {
        _onClose();
    }
    _channel.disableReadAndWrite();
    _socket.close();
}

void TcpConnection::close()
{
    onClose();
}