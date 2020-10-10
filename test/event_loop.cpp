#include "../src/event_loop.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(EventLopp, _eventLoop)
{
    GTEST_ASSERT_EQ(CurrentThread::_eventLoop, nullptr);
    EventLoop loop;
    GTEST_ASSERT_EQ(CurrentThread::_eventLoop, &loop);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
