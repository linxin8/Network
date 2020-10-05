#include "count_down_latch.h"

CountDownLatch::CountDownLatch(unsigned int count) : _mutex(), _condition(), _count(count) {}

void CountDownLatch::wait()
{
    std::unique_lock<std::mutex> unique_lock{_mutex};
    // wait之前需要lock，之后需要unlock
    _condition.wait(unique_lock, [this] { return _count == 0; });
}

void CountDownLatch::countDown()
{
    std::lock_guard<std::mutex> guard{_mutex};
    if (_count != 0)
    {
        _count -= 1;
    }
    if (_count == 0)
    {
        _condition.notify_all();
    }
}

int CountDownLatch::getCount() const
{
    std::lock_guard<std::mutex> guard{_mutex};
    return _count;
}