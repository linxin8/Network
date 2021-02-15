#include "../src/rpc_server.h"
#include "../../../src/tcp_server.h"
#include <cstring>

int add(int a, int b)
{
    return a + b;
}

std::string getTime()
{
    auto now = TimePoint::now();
    char str[60];
    sprintf(str, "%d:%d:%d", now.hours(), now.minute(), now.second());
    return str;
}

Message rpcAdd(Message arg)
{
    LOG_ASSERT(arg["fun"] == "add");
    Message reply;
    try
    {
        int  a          = std::stoi(arg["arg0"]);
        int  b          = std::stoi(arg["arg1"]);
        auto result     = add(a, b);
        reply["status"] = "ok";
        reply["result"] = std::to_string(result);
    }
    catch (std::out_of_range& e)
    {
        reply["status"] = "error";
        reply["error"]  = "argument out of range";
    }
    catch (std::invalid_argument& e)
    {
        reply["status"] = "error";
        reply["error"]  = "invalid argument";
    }
    catch (std::exception& e)
    {
        reply["status"] = "error";
        reply["error"]  = e.what();
    }
    return std::move(reply);
}

Message rpcGetTime(Message)
{
    return Message::createResultMessage(getTime());
}

int main()
{
    RPCServer server{2019};
    server.registerFuction("add", rpcAdd);
    server.registerFuction("add", rpcGetTime);
    while (true)
    {
        CurrentThread::sleep(1000);
    }
}