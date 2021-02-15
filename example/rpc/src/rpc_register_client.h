#pragma once
#include "rpc_client.h"
#include <set>

class RPCRegisterClient
{
public:
    RPCRegisterClient(std::string remoteIp, uint16_t remotePort);

public:
    void    onReadyToRead();
    Message onNewMessage(Message m);
    void    registerFun(std::string fun, std::string ip, uint16_t port);
    // return if fun existed
    bool        unregisterFun(std::string fun);
    InetAddress query(std::string fun);

private:
    void timer();

private:
    EventLoopThread      _timerThread;
    std::vector<Message> _registeredFun;
    RPCClient            _client;
    std::mutex           _mutex;
};