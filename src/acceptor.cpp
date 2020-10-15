#include "acceptor.h"
#include "event_loop.h"
#include "log.h"
#include <fcntl.h>
#include <unistd.h>

Acceptor::Acceptor(const InetAddress& listenAddress, bool isReusePort) :
    _onNewConnection{},
    _isListening{},
    _socket{true, false, true},
    _channel{0},
    _reserveFd{}
{
    _reserveFd = open("/dev/null", O_RDONLY | O_CLOEXEC);
    _socket.bind(listenAddress);
    _channel.setOnRead(std::bind(&Acceptor::onRead, this));
}

Acceptor::~Acceptor()
{
    _channel.disableRead();
    _channel.disableWrite();
    CurrentThread::getEventLoop().removeChannel(&_channel);
    close(_reserveFd);
}

void Acceptor::listen()
{
    _socket.listen();
}

void Acceptor::onRead()
{
    auto&& [ok, fd, address] = _socket.accept();
    if (ok)
    {
        if (_onNewConnection)
        {
            _onNewConnection(fd, address);
        }
        else
        {
            LOG_INFO() << "call back \"on new connection\" not ready ";
            close(fd);
        }
    }
    else
    {
        LOG_ERROR() << std::strerror(errno);
        if (errno == EMFILE)
        {
            ::close(_reserveFd);
            _reserveFd = ::accept(_socket.getFd(), NULL, NULL);
            ::close(_reserveFd);
            _reserveFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}