#include "../src/tcp_server.h"
#include "../src/event_loop_thread.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(TcpServer, resovle)
{
    TcpServer server{9000};
    server.setOnNewConnection([] { LOG_INFO() << "new connection"; });
    server.setOnReadyToRead([&server](int id) { server.close(id); });
    server.listen();
    while (true)
    {
        CurrentThread::sleep(200000);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
