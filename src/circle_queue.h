#pragma once
#include <cassert>
#include <cstddef>
#include <utility>

// not thread safe
template <typename T, size_t size>
class CircleQueue
{
    // test if is empty
    bool isEmpty()
    {
        return start == end;
    }
    // test if is full
    bool isFull()
    {
        return next(start) == end;
    }
    // remove and return the first item
    T take()
    {
        assert(!isEmpty());
        T&& ret = data[start];
        start   = next(start);
        return std::move(ret);
    }
    // add element
    void push(const T& x)
    {
        assert(!isFull());
        data[end] = x;
        end       = next(end);
    }

    // add element
    void add(T&& x)
    {
        assert(!isFull());
        data[end] = std::move(x);
        end       = next(end);
    }

private:
    int next(int x)
    {
        int ret = x + 1;
        if (ret >= size)
        {
            ret -= size;
        }
        return ret;
    }

private:
    T   data[size]{};
    int start = 0;
    int end   = 0;
};