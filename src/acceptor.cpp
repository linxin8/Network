#include "acceptor.h"
#include "event_loop.h"
#include "log.h"
#include <fcntl.h>
#include <unistd.h>

Acceptor::Acceptor(const InetAddress& listenAddress) :
    _onAcception{},
    _isListening{},
    _socket{true, false, true},
    _channel{_socket.getFd()},
    _reserveFd{}
{
    _reserveFd = open("/dev/null", O_RDONLY | O_CLOEXEC);
    _socket.setKeepAlive(true);
    _socket.setReuseAddress(true);
    _socket.setReusePort(true);
    _socket.bind(listenAddress);
    _channel.setOnRead(std::bind(&Acceptor::onAcception, this));
}

Acceptor::~Acceptor()
{
    _channel.disableReadAndWrite();
    close(_reserveFd);
}

void Acceptor::listen()
{
    assert(!_isListening);
    _socket.listen();
    _isListening = true;
    _channel.enableRead();
}

void Acceptor::onAcception()
{
    auto [ok, socket] = _socket.accept();
    if (ok)
    {
        if (_onAcception)
        {
            _onAcception(std::make_unique<TcpConnection>(std::move(socket)));
        }
        else
        {
            LOG_INFO() << "call back \"on new connection\" not ready ";
            // socket fd will atomatic close
        }
    }
    else
    {
        if (errno == EMFILE)
        {
            close(_reserveFd);
            _reserveFd = accept(_socket.getFd(), NULL, NULL);
            close(_reserveFd);
            _reserveFd = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}