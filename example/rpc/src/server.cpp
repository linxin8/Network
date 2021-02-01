#include "../../../src/tcp_server.h"

int64_t add(int64_t a, int64_t b)
{
    return a + b;
}

void onReadyToRead(std::shared_ptr<TcpConnection> con)
{
    LOG_DEBUG() << con->recvAll();
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