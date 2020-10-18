
#include "../src/acceptor.h"
#include "../src/event_loop.h"
#include "../src/event_loop_thread.h"
#include "../src/log.h"
#include "../src/thread.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(Acceptor, listen)
{
    InetAddress         address{9000};
    Acceptor            acceptor{address};
    std::vector<Socket> socketVector;
    acceptor.setOnAcception([&](Socket socket) {
        std::cout << "accept fd " << socket.getFd() << " local address "
                  << socket.getLocalAddress().getIpString() << " "
                  << socket.getLocalAddress().getPort() << " peer address "
                  << socket.getPeerAddress().getIpString() << " "
                  << socket.getPeerAddress().getPort() << std::endl;
        socketVector.push_back(std::move(socket));

        Channel channel{socket.getFd()};
        channel.setOnRead([]() {

        });
    });
    auto threadEntry = [&] {
        std::cout << "start listen " << address.getIpString() << " "
                  << address.getPort() << "\n";
        acceptor.listen();
    };

    EventLoopThread thread{threadEntry};
    thread.start();
    while (true)
    {
        CurrentThread::sleep(200);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
