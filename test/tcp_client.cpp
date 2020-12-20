#include "../src/tcp_client.h"
#include "../src/event_loop_thread.h"
#include "../src/log.h"
#include "../src/tcp_server.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(TcpClient, resovle)
{
    TcpClient client;
    auto [ok, address] = InetAddress::resolve("127.0.0.1", 9001);
    GTEST_ASSERT_EQ(ok, true);
    LOG_INFO() << address.getIp4String() << " " << address.getPort()
              << std::endl;
    client.setOnReadyToRead([&] {
        auto data = client.read();
        LOG_INFO() << "recv data: " << data;
    });
    ok = client.connectTo(address);
    GTEST_ASSERT_EQ(ok, true);
    client.send("23333");
    client.send("666");
    while (true)
    {
        CurrentThread::sleep(100);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
