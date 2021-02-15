#include "../src/rpc_client.h"

int main()
{
    RPCClient client{"localhost", 3000};
    int       a         = 0;
    int       b         = 1;
    auto      t1        = TimePoint::now();
    const int frequency = 100;
    int       n         = frequency;
    while (n--)
    {
        auto result = client.call(
            "add", {{"arg0", std::to_string(a)}, {"arg1", std::to_string(b)}});
        LOG_DEBUG() << result.toReadableString();
        int c = a + b;
        a     = c;
        b     = 1;
    }
    auto t2 = TimePoint::now();
    n       = frequency;
    std::vector<std::shared_ptr<RPCAsynResult>> result;
    while (n--)
    {
        auto r = client.callAsyn(
            "add", {{"arg0", std::to_string(a)}, {"arg1", std::to_string(b)}});
        result.push_back(r);
        int c = a + b;
        a     = c;
        b     = 1;
    }
    for (auto& r : result)
    {
        r->wait();
        LOG_DEBUG() << r->extract().toReadableString();
    }
    auto t3 = TimePoint::now();
    LOG_DEBUG() << "test case" << frequency << "call time"
                << t2.sub(t1).second() << "call asyn time"
                << t3.sub(t1).second();
    return 0;
}