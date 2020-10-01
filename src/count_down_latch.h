#pragma once
#include "type.h"
#include <condition_variable>
#include <mutex>

class CountDownLatch : Noncopyable
{
public:
    explicit CountDownLatch(unsigned int count);
    // wait untill count is euqal to 0
    void wait();
    void countDown();
    int  getCount() const;

private:
    mutable std::mutex      _mutex;
    std::condition_variable _condition;
    unsigned int            _count;
};