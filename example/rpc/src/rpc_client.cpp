
#include "rpc_client.h"
#include "../../../src/tcp_connection.h"
#include "message.h"
#include "serialization.h"
#include "utility.h"
void handleNewData(std::string data)
{
    static std::string rawData;
    rawData.append(data);
    while (Message::containsMessage(rawData))
    {
        auto m = Message::extractString(rawData);
        LOG_DEBUG() << m.toReadableString();
    }
}

void RPCClient::onNewMessage(Message message)
{
    LOG_DEBUG() << message.toReadableString();
    {
        std::lock_guard<std::mutex> guard{_mutex};
        _rcpResult = std::move(message);
        _condition.notify_one();
    }
    LOG_DEBUG() << "notify ";
}
void RPCClient::onReadyToRead()
{
    _client.read(_recvBuffer);
    while (Message::containsMessage(_recvBuffer))
    {
        Message m = Message::extractString(_recvBuffer);
        onNewMessage(std::move(m));
    }
};

Message RPCClient::call(std::string fun, Message arg)
{
    arg["fun"] = fun;
    auto ok    = _client.connectTo(_serverAddress);
    if (!ok)
    {
        return {{"status", "error"},
                {"error",
                 std::string("connot connect to server address ") +
                     _serverAddress.getIpString()}};
    }
    std::unique_lock<std::mutex> lock{_mutex};
    LOG_DEBUG() << "request " << arg.toString();
    _client.sendAsyn(arg.toString());
    _condition.wait(lock);
    LOG_DEBUG() << "get reply " << arg.toString();
    _client.disconnect();
    return std::move(_rcpResult);
}

RPCClient::RPCClient(std::string ip, uint16_t port) :
    _serverAddress{InetAddress::getInvalidAddress()},
    _client{},
    _recvBuffer{},
    _mutex{},
    _condition{},
    _rcpResult{}
{
    auto [ok, address] = InetAddress::resolve(ip, port);
    LOG_ASSERT(ok);
    _serverAddress = address;
    _client.setOnReadyToRead(std::bind(&RPCClient::onReadyToRead, this));
}
