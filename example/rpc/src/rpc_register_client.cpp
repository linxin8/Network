#include "rpc_register_client.h"
#include "message.h"
RPCRegisterClient::RPCRegisterClient(std::string localIp,
                                     uint16_t    localPort,
                                     std::string remoteIp,
                                     uint16_t    remotePort) :
    _client{std::move(remoteIp), remotePort},
    _localIp{std::move(_localIp)},
    _localPort{std::move(_localPort)}
{
}

void RPCRegisterClient::registerFun(std::string fun)
{
    Message m;
    m["register_fun"] = std::move(fun);
    m["ip"]           = "localhost";
    m["port"]         = std::to_string(2020);
    _client.call("register", std::move(m));
}

void RPCRegisterClient::unregisterFun(std::string fun)
{
    Message m;
    m["unregister_fun"] = fun;
    _client.call("unregister", std::move(m));
}

InetAddress RPCRegisterClient::query(std::string fun)
{
    Message m;
    m["query_fun"] = fun;
    _client.call("unregister", std::move(m));
}