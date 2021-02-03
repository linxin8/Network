#include "tcp_client.h"
#include <thread>

TcpClient::TcpClient() :
    _onConnected{},
    _onDisconnected{},
    _onReadyToRead{},
    _onError{},
    _onSend{},
    _recvBuffer{},
    _sendBuffer{},
    _thread{[] {}, "client network"},
    _isConnected{},
    _connection{}
{
    _thread.start();
}

bool TcpClient::connectTo(InetAddress address)
{
    Socket socket{true, false, true};
    bool   ok = socket.connect(address);
    if (ok)
    {
        _connection = std::make_shared<TcpConnection>(std::move(socket));
        _connection->setEventLoop(_thread.getEventLoop());
        _connection->enableRead();
        onConnected();
    }
    return ok;
}

void TcpClient::setOnConnected(std::function<void()> onConnected)
{
    _onConnected = std::move(onConnected);
}
void TcpClient::setOnDisconnected(std::function<void()> onDisconnected)
{
    _onDisconnected = std::move(onDisconnected);
}

void TcpClient::setOnReadyToRead(std::function<void()> onReadyToRead)
{
    _onReadyToRead = std::move(onReadyToRead);
}

void TcpClient::setOnError(std::function<void()> onError)
{
    _onError = std::move(onError);
}

void TcpClient::onConnected()
{
    _isConnected = true;
    _connection->setOnReadyToRead(std::bind(&TcpClient::onRead, this));
    _connection->setOnClose(std::bind(&TcpClient::onDisconnected, this));
    _connection->setOnError(
        std::bind(&TcpClient::onError, this, std::placeholders::_1));
    if (_onConnected)
    {
        _onConnected();
    }
}

void TcpClient::onDisconnected()
{
    _isConnected = false;
    if (_onDisconnected)
    {
        _onDisconnected();
    }
}

void TcpClient::onError(int errorNo)
{
    LOG_ERROR() << std::strerror(errorNo);
    if (_onError)
    {
        _onError();
    }
}

void TcpClient::onRead()
{
    char   buffer[2048];
    size_t size = 1;
    {
        // std::lock_guard<std::mutex> guard{_readWriteMutex};
        while (size != 0)
        {
            size = _connection->recv(buffer, 2048);
            _recvBuffer.append(buffer, size);
        }
    }
    if (_onReadyToRead)
    {
        _onReadyToRead();
    }
}

void TcpClient::send(const std::string data)
{
    LOG_ASSERT(_isConnected);
    if (data.empty())
    {
        return;
    }
    _thread.exec([con = _connection, d = std::move(data)] {
        con->sendAsyn(d.c_str(), d.size());
    });
}

std::string TcpClient::read()
{
    LOG_ASSERT(_isConnected);

    // while (_recvBuffer.size() == 0)
    // {
    //     std::this_thread::yield();
    // }

    std::string data;
    bool        ok = false;
    _thread.exec([&buffer = _recvBuffer, &data, &ok] {
        data = std::move(buffer);
        ok   = true;
    });

    while (!ok)
    {
        std::this_thread::yield();
    }
    return std::move(data);
}

void TcpClient::disconnect()
{
    _connection->close();
}