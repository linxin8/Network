#pragma once
#include <functional>

//监听端口，当有socket连接时，回调newConnection
class Acceptor
{
public:
    Acceptor();
    ~Acceptor();

private:
    void                  onNewConnection();
    std::function<void()> _onNewConnection;
};

Acceptor::Acceptor() {}

Acceptor::~Acceptor() {}
