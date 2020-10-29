#include "../src/tcp_server.h"
#include "../src/event_loop_thread.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(TcpServer, resovle)
{
    EventLoop loop;
    TcpServer server{9000};
    server.setThreadNumber(2);
    server.setOnReadyToRead([](std::shared_ptr<TcpConnection> connection) {
        char   buffer[100] = "echo back: ";
        size_t size        = connection->recv(buffer + strlen(buffer), 100);
        std::weak_ptr<TcpConnection> weakConnection = connection;
        connection->setOnSent([weakConnection](size_t) {
            if (auto con = weakConnection.lock())
            {
                con->close();
            }
        });
        connection->sendAsyn(buffer, strlen(buffer) + size);
    });
    server.listen();
    loop.start();
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
