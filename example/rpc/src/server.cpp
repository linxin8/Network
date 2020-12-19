#include "../../../src/tcp_server.h"

int64_t add(int64_t a, int64_t b)
{
    return a + b;
}

int main()
{
    EventLoop loop;
    TcpServer server{2019};
    server.listen();
}