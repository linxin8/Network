
#include "../src/acceptor.h"
#include "../src/event_loop.h"
#include "../src/event_loop_thread.h"
#include "../src/log.h"
#include "../src/thread.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(Acceptor, listen)
{
    InetAddress                                 address{9000};
    Acceptor                                    acceptor{address};
    std::vector<std::unique_ptr<TcpConnection>> connectionVector;
    char                                        buffer[2333];
    acceptor.setOnAcception([&](std::unique_ptr<TcpConnection> connection) {
        const auto& socket = connection->getSocket();
        std::cout << "accept fd " << socket.getFd() << " local address "
                  << socket.getLocalAddress().getIpString() << " "
                  << socket.getLocalAddress().getPort() << " peer address "
                  << socket.getPeerAddress().getIpString() << " "
                  << socket.getPeerAddress().getPort() << std::endl;
        auto index = connectionVector.size();
        connection->setOnReadyToRead([&, index](size_t size) {
            LOG_INFO() << "ready to read " << size;
            connectionVector[index]->recv(buffer, size);
            connectionVector[index]->sendAsyn(buffer, size);
        });
        connection->setOnSent(
            [&, index](size_t size) { connectionVector[index]->close(); });
        connectionVector.push_back(std::move(connection));
    });
    auto threadEntry = [&] {
        std::cout << "start listen " << address.getIpString() << " "
                  << address.getPort() << "\n";
        acceptor.listen();
    };

    EventLoopThread thread{threadEntry};
    thread.startLoop();
    while (true)
    {
        CurrentThread::sleep(2000);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
