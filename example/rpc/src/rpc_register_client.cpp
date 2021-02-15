#include "rpc_register_client.h"
#include "message.h"
RPCRegisterClient::RPCRegisterClient(std::string remoteIp,
                                     uint16_t    remotePort) :
    _client{remoteIp, remotePort}
{
    _timerThread.start();
    _timerThread.exec(std::bind(&RPCRegisterClient::timer, this));
}

void RPCRegisterClient::timer()
{
    const int period = 1000;
    while (true)
    {
        Message m;
        {
            std::lock_guard<std::mutex> guard{_mutex};
            for (auto& fun : _registeredFun)
            {
                _client.callAsyn("register", fun);
            }
        }
        CurrentThread::sleeps(period);
    }
}

void RPCRegisterClient::registerFun(std::string fun,
                                    std::string ip,
                                    uint16_t    port)
{
    Message m;
    m["register_fun"] = std::move(fun);
    m["ip"]           = ip;
    m["port"]         = std::to_string(port);
    {
        std::lock_guard<std::mutex> guard{_mutex};
        _registeredFun.emplace_back(std::move(m));
    }
}

bool RPCRegisterClient::unregisterFun(std::string fun)
{
    std::lock_guard<std::mutex> guard{_mutex};
    auto                        it =
        std::find_if(_registeredFun.begin(),
                     _registeredFun.end(),
                     [&fun](auto& x) { return x["register_fun"] == fun; });
    if (it != _registeredFun.end())
    {
        _registeredFun.erase(it);
        return true;
    }
    return false;
}

InetAddress RPCRegisterClient::query(std::string fun)
{
    Message m;
    m["query_fun"] = fun;
    m              = _client.call("unregister", std::move(m));
    LOG_ASSERT(m.contains("ip"));
    LOG_ASSERT(m.contains("port"));
    auto ip           = m["ip"];
    auto port         = std::stoi(m["port"]);
    auto&& [ok, addr] = InetAddress::resolve(ip, static_cast<uint16_t>(port));
    LOG_ASSERT(ok);
    return addr;
}