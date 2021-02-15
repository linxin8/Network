#include "../src/rpc_register_server.h"
#include <thread>

int main()
{
    RPCRegisterServer server{2000};
    while (true)
    {
        CurrentThread::sleeps(10);
    }
}