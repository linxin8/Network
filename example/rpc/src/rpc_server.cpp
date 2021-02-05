#include "rpc_server.h"
#include "../../../src/tcp_server.h"
#include "message.h"

RPCServer::RPCServer(uint16_t port) : _functionHandle{}, _server{port}
{
    _server.setOnReadyToRead(
        std::bind(&RPCServer::onReadyToRead, this, std::placeholders::_1));
    _server.listen();
}

void RPCServer::registerFuction(std::string                     name,
                                std::function<Message(Message)> handle)
{
    _functionHandle[name] = handle;
}

void RPCServer::onNewMessage(std::shared_ptr<TcpConnection> con,
                             Message                        message)
{
    LOG_DEBUG() << message.toReadableString();
    auto& fun = message["fun"];
    if (_functionHandle.contains(fun))
    {
        auto result = _functionHandle[fun](message.rawData());
        LOG_ASSERT(result.contains("status"));
        Message reply{std::move(result)};
        con->sendAsyn(reply.toString());
    }
    else
    {
        Message reply;
        reply["status"] = "error";
        reply["error"]  = std::string("function ") + fun + "not found";
        con->sendAsyn(message.toString());
    }
}

void RPCServer::onReadyToRead(std::shared_ptr<TcpConnection> con)
{
    auto& data = _recvBuffer[con];
    con->recv(data);
    while (Message::containsMessage(data))
    {
        Message m = Message::extractString(data);
        onNewMessage(con, std::move(m));
    }
};
