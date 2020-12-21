
#include "../../src/acceptor.h"
#include "../../src/event_loop.h"
#include "../../src/event_loop_thread.h"
#include "../../src/log.h"
#include "../../src/tcp_server.h"
#include "../../src/thread.h"
#include <iostream>

uint16_t port = 9001;

void serviceV2()
{
    TcpServer server{port};
    server.setSubReactorThreadNumber(2);
    std::vector<std::shared_ptr<TcpConnection>> cons;
    server.setOnReadyToRead([&](std::shared_ptr<TcpConnection> connection) {
        std::string data = "echo back info: ";
        char        buffer[100];
        size_t      size = connection->recv(buffer, 100);
        data.append(buffer, size);
        std::weak_ptr<TcpConnection> weakConnection = connection;
        // connection->setOnSent([weakConnection](size_t) {
        //     if (auto con = weakConnection.lock())
        //     {
        //         LOG_INFO() << "send done and close";
        //         con->close();
        //     }
        // });
        LOG_INFO() << "echo back: " << data;
        connection->sendAsyn(data.c_str(), data.size());
        cons.push_back(connection);  // avoid disconnecting
    });
    server.listen();
    while (true)
    {
        CurrentThread::sleep(1000);
    }
}

void service()
{
    InetAddress                                 address{port};
    Acceptor                                    acceptor{address};
    std::vector<std::shared_ptr<TcpConnection>> connectionVector;
    char                                        buffer[2333];
    acceptor.setOnAcception([&](std::shared_ptr<TcpConnection> connection) {
        const auto& socket = connection->getSocket();
        LOG_INFO() << "accept fd" << socket.getFd() << "local address"
                   << socket.getLocalAddress().getIpString()
                   << socket.getLocalAddress().getPort() << "peer address"
                   << socket.getPeerAddress().getIpString()
                   << socket.getPeerAddress().getPort();
        // auto index = connectionVector.size();
        // connection->setOnReadyToRead([&]() {
        //     size_t size = 2333;
        //     LOG_INFO() << "ready to read ";
        //     connectionVector[index]->recv(buffer, size);
        //     connectionVector[index]->sendAsyn("echo back: ", 11);
        //     connectionVector[index]->sendAsyn(buffer, size);
        // });
        // connection->setOnSent(
        //     [&, index](size_t size) { connectionVector[index]->close();
        //     });
        // connectionVector.push_back(std::move(connection));
    });
    auto threadEntry = [&] {
        LOG_INFO() << "start listen" << address.getIpString()
                   << address.getPort();
        acceptor.listen();
    };

    EventLoopThread thread{threadEntry};
    thread.start();
    while (true)
    {
        CurrentThread::sleep(100);
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        std::string number = argv[1];
        port               = std::stoi(number);
    }
    serviceV2();
    return 0;
}
