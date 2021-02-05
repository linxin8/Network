#pragma once

#include "../../../src/tcp_connection.h"
#include "../../../src/tcp_server.h"
#include "message.h"
#include "utility.h"
#include <functional>
#include <map>
#include <memory>
#include <string>

class RPCServer
{
public:
    RPCServer(uint16_t port);
    // set or update
    void registerFuction(std::string                     name,
                         std::function<Message(Message)> handle);

private:
    void onReadyToRead(std::shared_ptr<TcpConnection> con);
    void onNewMessage(std::shared_ptr<TcpConnection> con, Message message);

private:
    std::map<std::string, std::function<Message(Message)>> _functionHandle;
    std::map<std::shared_ptr<TcpConnection>, std::string>  _recvBuffer;
    TcpServer                                              _server;
};