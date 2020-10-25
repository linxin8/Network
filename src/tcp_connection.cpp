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
    _channel.setOnError(std::bind(&TcpConnection::onError, this));
    _channel.setOnRead(std::bind(&TcpConnection::onRead, this));
    _channel.setOnWrite(std::bind(&TcpConnection::onWrite, this));
    _channel.enableRead();
}

size_t TcpConnection::sendAsyn(const void* data, size_t size)
{
    if (size > 0 && !_channel.isEnbleWrite())
    {
        _channel.enableWrite();
    }
    return _sendBuffer.append(data, size);
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

void TcpConnection::onRead()
{
    auto [address, maxSize] = _recvBuffer.beginDirectWrite();
    ssize_t size            = _socket.recvNonblocking(address, maxSize);
    _recvBuffer.endDirectWrite(std::max(static_cast<ssize_t>(0), size));
    if (size == -1)
    {
        onError();
    }
    else if (size > 0)
    {
        if (_onReadyToRead)
        {
            _onReadyToRead(_recvBuffer.getSize());
        }
    }
}

void TcpConnection::onWrite()
{
    size_t size = _socket.sendNonblocking(_sendBuffer.getFirstAddress(),
                                          _sendBuffer.getContinousSize());
    if (size == -1)
    {
        onError();
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

void TcpConnection::onError()
{
    LOG_ERROR() << std::strerror(errno);
}

void TcpConnection::onClose()
{
    _channel.disableReadAndWrite();
    if (_onClose)
    {
        _onClose();
    }
    // not close fd
}

void TcpConnection::close()
{
    _channel.disableReadAndWrite();
    _socket.close();
}