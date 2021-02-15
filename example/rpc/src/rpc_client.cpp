
#include "rpc_client.h"
#include "../../../src/tcp_connection.h"
#include "message.h"
#include "serialization.h"
#include "utility.h"
#include <queue>

class ClientPool
{
public:
    static std::shared_ptr<TcpClient> get()
    {
        std::lock_guard<std::mutex> gurad{_mutex};
        std::shared_ptr<TcpClient>  client;
        if (_avaliableClient.empty())
        {
            client = std::make_shared<TcpClient>();
        }
        else
        {
            client = _avaliableClient.front();
            _avaliableClient.pop_front();
            LOG_ASSERT(!_runningClient.count(client));
        }
        _runningClient.insert(client);
        return client;
    }

    static void release(std::shared_ptr<TcpClient> client)
    {
        std::lock_guard<std::mutex> gurad{_mutex};
        LOG_ASSERT(_runningClient.count(client));
        _runningClient.erase(client);
        _avaliableClient.push_back(client);
    }

private:
    inline static std::set<std::shared_ptr<TcpClient>>   _runningClient;
    inline static std::deque<std::shared_ptr<TcpClient>> _avaliableClient;
    inline static std::mutex                             _mutex;
};

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

Message RPCClient::call(std::string fun, Message arg)
{
    auto           client = ClientPool::get();
    Message        result;
    std::string    buffer;
    CountDownLatch latch{1};
    client->setOnReadyToRead([&]() {
        client->read(buffer);
        if (Message::containsMessage(buffer))
        {
            result = Message::extractString(buffer);
            client->disconnect();
            ClientPool::release(client);
            latch.countDown();
        }
    });
    auto ok = client->connectTo(_serverAddress);
    if (!ok)
    {
        ClientPool::release(client);
        return {{"status", "error"},
                {"error",
                 std::string("connot connect to server address ") +
                     _serverAddress.getIpString()}};
    }
    arg["fun"] = fun;
    client->sendAsyn(arg.toString());
    latch.wait();
    return std::move(result);
}

std::shared_ptr<RPCAsynResult> RPCClient::callAsyn(std::string fun, Message arg)
{
    std::shared_ptr<TcpClient>     client = ClientPool::get();
    std::shared_ptr<RPCAsynResult> result = std::make_shared<RPCAsynResult>();
    std::weak_ptr<RPCAsynResult>   weakResult = result;
    client->setOnReadyToRead(
        [client, buffer = std::string(), weakResult]() mutable {
            client->read(buffer);
            if (Message::containsMessage(buffer))
            {
                Message m = Message::extractString(buffer);
                if (auto task = weakResult.lock())
                {
                    task->setResult(std::move(m));
                }
                client->disconnect();
                ClientPool::release(client);
            }
        });

    auto ok = client->connectTo(_serverAddress);
    if (!ok)
    {
        Message m{{"status", "error"},
                  {"error",
                   std::string("connot connect to server address ") +
                       _serverAddress.getIpString()}};
        result->setResult(std::move(m));
        ClientPool::release(client);
        return result;
    }
    arg["fun"] = fun;
    client->sendAsyn(arg.toString());
    return result;
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
}
