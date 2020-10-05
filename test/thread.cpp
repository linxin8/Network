#include "../src/thread.h"
#include <gtest/gtest.h>
#include <iostream>

void threadMain()
{
    std::cout << "name: " << CurrentThread::getName() << " tid: " << CurrentThread::getTidString()
              << " isMainThread: " << CurrentThread::isMainThread() << std::endl;
    GTEST_ASSERT_EQ(std::to_string(CurrentThread::getTid()), CurrentThread::getTidString());
    GTEST_ASSERT_EQ(CurrentThread::isMainThread(), false);
}

TEST(Thread, threadVariable)
{
    Thread t{threadMain};
    GTEST_ASSERT_EQ(t.isStarted(), false);
    t.start();
    GTEST_ASSERT_EQ(t.isStarted(), true);
    t.join();
}

TEST(Thread, createMultibleThread)
{
    for (int i = 0; i < 10; i++)
    {
        Thread t{threadMain};
        GTEST_ASSERT_EQ(t.isStarted(), false);
        t.start();
        GTEST_ASSERT_EQ(t.isStarted(), true);
        t.join();
    }
}

TEST(Thread, dataRace)
{
    int num = 0;
    for (int i = 0; i < 10; i++)
    {
        Thread t{[&num] { num++; }};
        t.start();
        t.join();
    }
    GTEST_ASSERT_EQ(num, 10);
    for (int i = 0; i < 10; i++)
    {
        Thread t{[&num] { num--; }};
        t.start();
        t.join();
    }
    GTEST_ASSERT_EQ(num, 0);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    assert(CurrentThread::isMainThread());
    return RUN_ALL_TESTS();
}
