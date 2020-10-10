#include "../src/inet_address.h"
#include "../src/file.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(InetAddress, resovle)
{
    auto&& [ok, address] = InetAddress::resolve("baidu.com", 80);
    if (ok)
    {
        std::cout << "resolve baidu.com :" << address.getIpString() << std::endl;
    }
    else
    {
        std::cout << "resolve failed" << std::endl;
    }
    GTEST_ASSERT_EQ(ok, true);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
