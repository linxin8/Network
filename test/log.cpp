#include "../src/log.h"
#include "../src/core_dump.h"
#include <charconv>
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <iostream>

TEST(FixedByteBuffer, append)
{
    FixedByteBuffer<200> buffer;
    buffer.append(2.f);
    buffer.append('\0');
    GTEST_ASSERT_EQ(2.f, std::atof(buffer.toCString()));
    buffer.clear();
    buffer.append(2.9);
    buffer.append('\0');
    GTEST_ASSERT_EQ(2.9, std::atof(buffer.toCString()));
    buffer.clear();
    buffer.append(2);
    buffer.append('\0');
    GTEST_ASSERT_EQ(2, std::atoi(buffer.toCString()));
    buffer.clear();
    buffer.append(231123l);
    buffer.append('\0');
    GTEST_ASSERT_EQ(231123l, std::atol(buffer.toCString()));
    buffer.clear();
    buffer.append(231123231123ll);
    buffer.append('\0');
    GTEST_ASSERT_EQ(231123231123ll, std::atoll(buffer.toCString()));
    for (int i = 0; i < 100; i++)
    {
        buffer.append(i);
    }
    GTEST_ASSERT_EQ(buffer.isOverflow(), true);
    buffer.clear();
    GTEST_ASSERT_EQ(buffer.isOverflow(), false);
}

TEST(LoggerBuffer, op)
{
    LoggerBuffer buffer;
    const char   str[20] = "abcd";
    buffer << "123"
           << "345" << 789 << str << '\0' << ' ' << "!@#$" << 2.3f << 2.4 << 111ull;
}

void logFun(AsyncLogger& asyn, bool& ok)
{
    char data[100];
    for (int i = 0; i < 100000000 && ok; i++)
    {
        // auto result = std::to_chars(data, data + 100, i);
        // int  size   = result.ptr - data;
        // data[size]  = '\n';
        // asyn.append(data, size);
        // std::this_thread::sleep_for(std::chrono::microseconds{1});
        asyn.append("12345678910!@"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111#$%^&",
                    200);
        // std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    printf("end \n");
}

TEST(AsynLoggerBuffer, run)
{
    auto file = std::freopen("stderr.txt", "w", stderr);
    (void*)file;
    LoggerBuffer buffer;
    const char   str[20] = "abcd";
    buffer << "123"
           << "345" << 789 << str << '\0' << ' ' << "!@#$" << 2.3f << 2.4 << 111ull;
    AsyncLogger asyn;
    asyn.start();
    bool ok      = true;
    auto future  = std::async(std::launch::async, [&] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        printf("set ok false\n");
        ok = false;
    });
    auto future1 = std::async(std::launch::async, std::bind(&logFun, std::ref(asyn), std::ref(ok)));
    auto future2 = std::async(std::launch::async, std::bind(&logFun, std::ref(asyn), std::ref(ok)));
    auto future3 = std::async(std::launch::async, std::bind(&logFun, std::ref(asyn), std::ref(ok)));
    auto future4 = std::async(std::launch::async, std::bind(&logFun, std::ref(asyn), std::ref(ok)));
    // while (ok)
    // {
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }
}

int main(int argc, char* argv[])
{
    enable_core_dump();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
