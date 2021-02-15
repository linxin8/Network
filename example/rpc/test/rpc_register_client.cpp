#include "../src/rpc_register_client.h"
#include "../src/rpc_server.h"
#include <thread>
int main()
{
    RPCRegisterClient client{"localhost", 2000};
    client.registerFun("add", "localhost", 3000);

    RPCServer server{3000};
    server.registerFuction("add", [](Message message) {
        auto a = std::stoi(message["arg0"]);
        auto b = std::stoi(message["arg1"]);
        auto c = a + b;
        return Message::createResultMessage(std::to_string(c));
    });

    while (true)
    {
        CurrentThread::sleeps(1000);
    }
}