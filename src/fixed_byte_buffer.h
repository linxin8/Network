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
#include <string>

// a appendable buffer, allow retreive from the begining of raw data.
// when append the data that size is large than available size, no data is
// appended. and it step into overflow state that cannot append any data
// until clear function is called.
template <size_t N>
class FixedByteBuffer : Noncopyable
{
public:
    // total size
    constexpr size_t capacity() const
    {
        return N;
    }
    // the size of held elements
    size_t size() const
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
    void append(char* str)
    {
        append(static_cast<void*>(str), strlen(str));
    }

    // append data. if in overflow state, do nothing.
    template <typename T>
    void append(const T& x)
    {
        if (_isOverflow)
        {
            return;
        }
        std::to_chars_result&& result =
            std::to_chars(_data + _size, _data + N, x);
        if (result.ec == std::errc::value_too_large)
        {
            _isOverflow = true;
        }
        else
        {
            _size = result.ptr - _data;
        }
    }

    // append data. if in overflow state, do nothing.
    void append(const std::string& x)
    {
        append(x.c_str(), x.length());
    }

    // append data. if in overflow state, do nothing.
    void append(bool x)
    {
        x ? append("true", 4) : append("false", 5);
    }

    template <typename T>
    void append(T* x)
    {
        append(static_cast<const void*>(x));
    }

    // append pointer, start by 0x prefix
    void append(const void* x)
    {
        constexpr int size = sizeof(void*) + 2;
        if (_isOverflow)
        {  // cannot hold any message
            return;
        }
        append("0x", 2);
        std::to_chars_result&& result = std::to_chars(
            _data + _size, _data + N, reinterpret_cast<size_t>(x), 16);
        if (result.ec == std::errc::value_too_large)
        {
            _isOverflow = true;
        }
        else
        {
            _size = result.ptr - _data;
        }
    }

    // append data. if in overflow state, do nothing.
    void append(double x)
    {
        char str[32];  // double string length can not excesses 32
        int size = sprintf(str, "%lf", x);  // to do, wait for implementation of
                                            // std::to_chars on double argument
        append(str, size);
    }

    // append data. if in overflow state, do nothing.
    void append(float x)
    {
        char str[32];  // double string length can not excesses 32
        int  size = sprintf(str, "%f", x);  // to do, wait for implementation of
                                            // std::to_chars on float argument
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

    // once it can not append data, it changed into overflow state and cannot
    // append data any more.
    bool isOverflow() const
    {
        return _isOverflow;
    }

    // inner data
    const void* rowData() const
    {
        return _data;
    }

    // inner data in string format
    const char* toCString() const
    {
        return static_cast<const char*>(_data);
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