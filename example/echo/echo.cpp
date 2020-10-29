
#include "../../src/acceptor.h"
#include "../../src/event_loop.h"
#include "../../src/event_loop_thread.h"
#include "../../src/log.h"
#include "../../src/thread.h"
#include <iostream>

void service()
{
    InetAddress                                 address{9000};
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
        auto index = connectionVector.size();
        // connection->setOnReadyToRead([&, index](size_t size) {
        //     LOG_INFO() << "ready to read " << size;
        //     connectionVector[index]->recv(buffer, size);
        //     connectionVector[index]->sendAsyn("echo back: ", 11);
        //     connectionVector[index]->sendAsyn(buffer, size);
        // });
        connection->setOnSent(
            [&, index](size_t size) { connectionVector[index]->close(); });
        connectionVector.push_back(std::move(connection));
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
    service();
    return 0;
}
