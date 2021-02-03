#include "../../../src/tcp_client.h"
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

int main()
{
    TcpClient client;
    auto [ok, serverAddress] = InetAddress::resolve("localhost", 2019);
    assert(ok);
    LOG_DEBUG() << serverAddress.getIpString();
    client.setOnConnected([] { LOG_DEBUG() << "connected"; });
    client.setOnDisconnected([] { LOG_DEBUG() << "disconnected"; });
    client.setOnReadyToRead([&] { handleNewData(client.read()); });
    client.connectTo(serverAddress);
    Message message;
    message["action"] = "add";
    message["arg0"]   = "123";
    message["arg1"]   = "456";
    for (int i = 0; i < 10; i++)
    {
        message["id"] = std::to_string(generateID());
        client.send(message.toString());
    }
    message["action"] = "getTime";
    client.send(message.toString());
    CurrentThread::sleeps(2);
    return 0;
}
