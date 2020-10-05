#include "../src/log.h"
#include "../src/core_dump.h"
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

TEST(AsynLoggerBuffer, run)
{
    LoggerBuffer buffer;
    const char   str[20] = "abcd";
    buffer << "123"
           << "345" << 789 << str << '\0' << ' ' << "!@#$" << 2.3f << 2.4 << 111ull;
    AsyncLogger asyn;
    asyn.start();
    for (int i = 0; i < 100; i++)
    {
        auto str = std::to_string(i);
        asyn.append(str.c_str(), str.size());
        asyn.append("\n", 1);
    }
    asyn.stop();
}

int main(int argc, char* argv[])
{
    enable_core_dump();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
