#pragma once
#include "../../../src/tcp_client.h"
#include "message.h"
#include "utility.h"

class RPCClient
{
public:
    RPCClient(std::string ip, uint16_t port);

    // block until reply message is received
    Message call(std::string fun, Message arg);

private:
    void onReadyToRead();
    void onNewMessage(Message message);

private:
    InetAddress             _serverAddress;
    TcpClient               _client;
    std::string             _recvBuffer;
    std::mutex              _mutex;
    std::condition_variable _condition;
    Message                 _rcpResult;
};