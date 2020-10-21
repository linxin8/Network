#include "../src/tcp_buffer.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(TcpBuffer, appendInt)
{
    TcpBuffer buffer;
    for (int i = 0; i < 10; i++)
    {
        buffer.append(i);
    }
    for (int i = 0; i < 10; i++)
    {
        auto value = buffer.take<int>();
        GTEST_ASSERT_EQ(value, i);
    }
}

TEST(TcpBuffer, appendPod)
{
#pragma pack(push, 1)
    struct Pod
    {
        int32_t x;
        int8_t  y;
        double  c;
        char    d;
    };
    static_assert(sizeof(Pod) == 14);
#pragma pack(pop)
    TcpBuffer buffer;
    Pod       pod{.x = 1, .y = 2, .c = 3.0, .d = 4};
    buffer.append(pod);
    auto value = buffer.take<Pod>();
    GTEST_ASSERT_EQ(pod.x, value.x);
    GTEST_ASSERT_EQ(pod.y, value.y);
    GTEST_ASSERT_EQ(pod.c, value.c);
    GTEST_ASSERT_EQ(pod.d, value.d);
}

TEST(TcpBuffer, appendMix)
{
#pragma pack(push, 1)
    struct Pod
    {
        int32_t        x;
        short          y;
        double         c;
        char           d;
        constexpr bool operator==(const Pod& pod) const
        {
            return x == pod.x && y == pod.y && c == pod.c && d == pod.d;
        }
    };
    static_assert(sizeof(Pod) == 15);
#pragma pack(pop)
    Pod       pod{.x = 1, .y = 2, .c = 3.0, .d = 4};
    TcpBuffer buffer;
    buffer.append(123);
    buffer.append(pod);
    buffer.append(true);
    buffer.append(pod);

    GTEST_ASSERT_EQ(123, buffer.take<int>());
    GTEST_ASSERT_EQ(pod, buffer.take<Pod>());
    GTEST_ASSERT_EQ(true, buffer.take<bool>());
    GTEST_ASSERT_EQ(1, buffer.take<int32_t>());
    GTEST_ASSERT_EQ(2, buffer.take<short>());
    GTEST_ASSERT_EQ(3, buffer.take<double>());
    GTEST_ASSERT_EQ(4, buffer.take<char>());
}

TEST(TcpBuffer, smallBuffer)
{
    TcpBuffer buffer;
    for (int i = 1; i < 10; i++)
    {
        buffer.append(i);
        buffer.append(i);
    }
    for (int i = 1; i < 10; i++)
    {
        auto x = buffer.take<int>();
        buffer.popN(sizeof(x));
        GTEST_ASSERT_EQ(x, i);
    }
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
