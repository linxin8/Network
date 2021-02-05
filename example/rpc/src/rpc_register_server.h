#pragma once
#include "rpc_server.h"

class RPCRegisterServer
{
    struct Info
    {
        std::string ip;
        std::string port;
        TimePoint   expirationTime;
        Info() = default;
        Info(std::string ip, std::string port, TimePoint expirationTime) :
            ip{std::move(ip)},
            port{std::move(port)},
            expirationTime{expirationTime}
        {
        }
    };

public:
    RPCRegisterServer(uint16_t port);

private:
    void    onReadyToRead();
    Message onNewMessage(Message m);
    Message resiger(Message message);
    Message query(Message message);

private:
    RPCServer                   _server;
    std::map<std::string, Info> _addressMap;
};