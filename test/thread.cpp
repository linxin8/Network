#include "../src/thread.h"
#include <gtest/gtest.h>
#include <iostream>

void threadMain()
{
    std::cout << "name: " << CurrentThread::getName() << " tid: " << CurrentThread::getTidString()
              << " isMainThread: " << CurrentThread::isMainThread() << std::endl;
    GTEST_ASSERT_EQ(CurrentThread::isMainThread(), false);
}

TEST(Thread, variable)
{
    Thread t{threadMain};
    t.start();
    t.join();
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
