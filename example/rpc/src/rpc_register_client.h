#pragma once
#include "rpc_client.h"

class RPCRegisterClient
{
    RPCRegisterClient(std::string localIp,
                      uint16_t    localPort,
                      std::string remoteIp,
                      uint16_t    remotePort);

public:
    void        onReadyToRead();
    Message     onNewMessage(Message m);
    void        registerFun(std::string fun);
    void        unregisterFun(std::string fun);
    InetAddress query(std::string fun);

private:
    RPCClient   _client;
    std::string _localIp;
    uint16_t    _localPort;
};