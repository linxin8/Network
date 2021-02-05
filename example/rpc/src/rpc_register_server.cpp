#include "rpc_register_server.h"

RPCRegisterServer::RPCRegisterServer(uint16_t port) : _server{port}
{
    _server.registerFuction(
        "register",
        std::bind(&RPCRegisterServer::resiger, this, std::placeholders::_1));
    _server.registerFuction(
        "query",
        std::bind(&RPCRegisterServer::query, this, std::placeholders::_1));
}

Message RPCRegisterServer::resiger(Message message)
{
    LOG_ASSERT(message["fun"] == "register");
    if (!message.contains("register_fun"))
    {
        return Message::createErrorMessage("register_fun not found");
    }
    if (!message.contains("ip"))
    {
        return Message::createErrorMessage("ip not found");
    }
    if (!message.contains("port"))
    {
        return Message::createErrorMessage("port not found");
    }
    _addressMap.emplace(
        message["fun"],
        Info(message["ip"], message["port"], TimePoint::now().add(60)));
    return Message::createResultMessage("success");
}

Message RPCRegisterServer::query(Message message)
{
    LOG_ASSERT(message["fun"] == "query");

    if (!message.contains("query_fun"))
    {
        return Message::createErrorMessage("query_fun not found");
    }

    auto& funName = message["query_fun"];
    if (!_addressMap.count(funName))
    {
        return Message::createErrorMessage("address not exist");
    }
    auto& info = _addressMap[funName];
    return {{"status", "ok"}, {"ip", info.ip}, {"port", info.port}};
}