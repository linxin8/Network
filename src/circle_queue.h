#pragma once
#include <cassert>
#include <cstddef>
#include <utility>

// not thread safe
template <typename T, size_t size>
class CircleQueue
{
    static_assert(size > 1);

public:
    // test if is empty
    bool isEmpty() const
    {
        return _length == 0;
    }
    // test if is full
    bool isFull() const
    {
        return _length == size;
    }
    // remove and return the first item
    T take()
    {
        assert(!isEmpty());
        T& ret = _data[_start];
        _start = next(_start);
        _length--;
        return std::move(ret);
    }
    // add element
    void push(const T& x)
    {
        assert(!isFull());
        _data[_end] = x;
        _end        = next(_end);
        _length++;
    }

    // add element
    void push(T&& x)
    {
        assert(!isFull());
        _data[_end] = std::move(x);
        _end        = next(_end);
        _length++;
    }

private:
    int next(int x) const
    {
        int ret = x + 1;
        if (ret >= size)
        {
            ret -= size;
        }
        return ret;
    }

private:
    T   _data[size]{};
    int _start  = 0;
    int _end    = 0;
    int _length = 0;
};