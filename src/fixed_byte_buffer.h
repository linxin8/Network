#pragma once
#include "type.h"
#include <cassert>
#include <charconv>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

template <size_t N>
class FixedByteBuffer : Noncopyable
{
public:
    // total size
    constexpr size_t capacity()
    {
        return N;
    }
    // the size of held elements
    size_t size()
    {
        return _size;
    }

    // the size elements that can be held
    size_t availableSize()
    {
        return N - _size;
    }

    // append fixed size data. if in overflow state, do nothing.
    void append(const void* data, size_t size)
    {
        if (_isOverflow)
        {  // cannot hold any message
            return;
        }
        if (N - _size < size)
        {
            _isOverflow = true;
            return;
        }
        memcpy(&_data[_size], data, size);
        _size += size;
    }
    // append data. if in overflow state, do nothing.
    void append(const char* str)
    {
        append(static_cast<const void*>(str), strlen(str));
    }

    // append data. if in overflow state, do nothing.
    template <typename T>
    void append(T x)
    {
        if (_isOverflow)
        {
            return;
        }
        std::to_chars_result&& result = std::to_chars(_data + _size, _data + N, x);
        if (result.ec == std::errc::value_too_large)
        {
            _isOverflow = true;
        }
        else
        {
            _size = result.ptr - _data;
        }
    }

    // append pointer, start by 0x prefix
    void append(void* x)
    {
        constexpr int size = 22;
        if (_isOverflow)
        {  // cannot hold any message
            return;
        }
        if (N - _size < size)
        {
            _isOverflow = true;
            return;
        }
        append("0x", 2);
        append(static_cast<size_t>(size));
    }

    // append data. if in overflow state, do nothing.
    void append(double x)
    {
        char str[32];                        // double string length can not excesses 32
        int  size = sprintf(str, "%lf", x);  // to do, wait for implementation of std::to_chars on double argument
        append(str, size);
    }

    // append data. if in overflow state, do nothing.
    void append(float x)
    {
        char str[32];                       // double string length can not excesses 32
        int  size = sprintf(str, "%f", x);  // to do, wait for implementation of std::to_chars on float argument
        append(str, size);
    }

    // append data. if in overflow state, do nothing.
    void append(char x)
    {
        if (_size + 1 > N)
        {
            _isOverflow = true;
        }
        if (_isOverflow)
        {
            return;
        }
        _data[_size] = x;
        _size++;
    }

    // once it can not append data, it changed into overflow state and cannot append data any more.
    bool isOverflow()
    {
        return _isOverflow;
    }

    // inner data
    void* rowData()
    {
        return _data;
    }

    // inner data in string format
    char* toCString()
    {
        return static_cast<char*>(_data);
    }

    // reinterpret buffer as T object
    template <typename T>
    T& reinterpret()
    {
        return *reinterpret_cast<T*>(&_data);
    }

    // clear buffer, reset status
    void clear()
    {
        _size       = 0;
        _isOverflow = false;
    }

private:
    char   _data[N];
    size_t _size       = 0;
    bool   _isOverflow = false;
};