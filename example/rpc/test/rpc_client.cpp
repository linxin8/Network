#include "../src/rpc_client.h"

int main()
{
    RPCClient client{"localhost", 2019};
    int       a = 0;
    int       b = 1;
    int       n = 10000;
    while (n--)
    {
        auto result = client.call(
            "add", {{"arg0", std::to_string(a)}, {"arg1", std::to_string(b)}});
        LOG_DEBUG() << result.toReadableString();
        int c = a + b;
        a     = c;
        b     = 1;
    }
    return 0;
}