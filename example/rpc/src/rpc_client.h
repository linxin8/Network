#pragma once
#include "../../../src/tcp_client.h"
#include "message.h"
#include "utility.h"
#include <set>

class RPCAsynResult
{
public:
    RPCAsynResult() : _ok{false}, _extracted{false}, _message{}, _mutex{} {}
    ~RPCAsynResult() {}

    void setResult(Message message)
    {
        std::lock_guard<std::mutex> guard{_mutex};
        LOG_ASSERT(!_ok);
        LOG_ASSERT(!_extracted);
        _message = std::move(message);
        _ok      = true;
        _condition.notify_one();
    }

    Message extract()
    {
        std::lock_guard<std::mutex> guard{_mutex};
        LOG_ASSERT(_ok);
        LOG_ASSERT(!_extracted);
        _extracted = true;
        return std::move(_message);
    }

    // block until result is set
    void wait()
    {
        std::unique_lock<std::mutex> lock{_mutex};
        LOG_ASSERT(!_extracted);
        if (!_ok)
        {
            _condition.wait(lock);
        }
    }

    bool hasResult()
    {
        std::lock_guard<std::mutex> guard{_mutex};
        return _ok;
    }

private:
    bool                    _ok;
    bool                    _extracted;
    Message                 _message;
    std::mutex              _mutex;
    std::condition_variable _condition;
};

class RPCClient
{
public:
    RPCClient(std::string ip, uint16_t port);

    // block until reply message is received
    Message call(std::string fun, Message arg);

    std::shared_ptr<RPCAsynResult> callAsyn(std::string fun, Message arg);

private:
    InetAddress                              _serverAddress;
    TcpClient                                _client;
    std::string                              _recvBuffer;
    std::mutex                               _mutex;
    std::condition_variable                  _condition;
    Message                                  _rcpResult;
    std::set<std::shared_ptr<RPCAsynResult>> _resultSet;
};