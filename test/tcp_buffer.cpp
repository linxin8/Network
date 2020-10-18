#include "../src/tcp_buffer.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(TcpBUffer, appendInt)
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

TEST(TcpBUffer, appendPod)
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

TEST(TcpBUffer, appendMix)
{
#pragma pack(push, 1)
    struct Pod
    {
        int32_t        x;
        int8_t         y;
        double         c;
        char           d;
        constexpr bool operator==(const Pod& pod) const
        {
            return x == pod.x && y == pod.y && c == pod.c && d == pod.d;
        }
    };
    static_assert(sizeof(Pod) == 14);
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
    GTEST_ASSERT_EQ(pod, buffer.take<Pod>());
}

TEST(TcpBUffer, readIndex)
{
    TcpBuffer buffer;
    for (int i = 10; i < 20; i++)
    {
        buffer.append(i);
    }
    GTEST_ASSERT_EQ(buffer.getReadIndex(), 0);
    GTEST_ASSERT_EQ(buffer.getSize(), 10 * sizeof(int));
    GTEST_ASSERT_EQ(buffer.take<int>(), 10);
    GTEST_ASSERT_EQ(buffer.getReadIndex(), sizeof(int));
    buffer.seekReadOffset(sizeof(int));
    GTEST_ASSERT_EQ(buffer.getReadIndex(), sizeof(int) * 2);
    GTEST_ASSERT_EQ(buffer.take<int>(), 12);
    buffer.rewriteFront(123);
    buffer.seekReadBegin();
    GTEST_ASSERT_EQ(buffer.getReadIndex(), 0);
    GTEST_ASSERT_EQ(buffer.take<int>(), 123);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
