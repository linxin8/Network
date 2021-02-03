#include "../../../src/tcp_server.h"
#include "message.h"

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

void onNewMessage(std::shared_ptr<TcpConnection> con, Message message)
{
    LOG_DEBUG() << message.toReadableString();
    if (message["action"] == "add")
    {
        auto    a   = std::stoi(message["arg0"]);
        auto    b   = std::stoi(message["arg1"]);
        auto    res = add(a, b);
        Message reply;
        reply["status"] = "ok";
        reply["result"] = std::to_string(res);
        auto raw        = reply.toString();
        con->sendAsyn(raw.c_str(), raw.size());
    }
    else if (message["action"] == "getTime")
    {
        Message reply;
        reply["status"] = "ok";
        reply["result"] = getTime();
        auto raw        = reply.toString();
        con->sendAsyn(raw.c_str(), raw.size());
    }
}

std::map<std::shared_ptr<TcpConnection>, std::string> recvData;

void onReadyToRead(std::shared_ptr<TcpConnection> con)
{
    auto&       data    = recvData[con];
    std::string newData = con->recvAll();
    data.append(newData);
    int n = 0;
    while (Message::containsMessage(data))
    {
        Message m = Message::extractString(data);
        onNewMessage(con, std::move(m));
    }
};

int main()
{
    EventLoop loop;
    TcpServer server{2019};
    server.setOnReadyToRead(onReadyToRead);
    server.listen();
    while (true)
    {
        CurrentThread::sleep(1000);
    }
}