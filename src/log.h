#pragma once
#include "fixed_byte_buffer.h"
#include <bitset>
#include <condition_variable>
#include <memory>
#include <mutex>

class AsyncLogging : Noncopyable
{
};

enum Level
{
    trace,
    debug,
    info,
    error
};

class Logger
{
public:
    Logger(const char* file, const char* function, int line);
    template <typename T>
    Logger& operator<<(const T& data)
    {
        return *this;
    }
};

class LoggerBuffer : Noncopyable
{
public:
    LoggerBuffer() = default;
    LoggerBuffer(const char* file, const char* function, int line);
    ~LoggerBuffer() = default;
    template <typename T>
    LoggerBuffer& operator<<(T x)
    {
        _buffer.append(x);
        return *this;
    }

private:
    FixedByteBuffer<1000> _buffer;
};

template <size_t N>
class LogBufferPool : Noncopyable
{
    LoggerBuffer& get()
    {
        int bit = -1;
        if (_queue.start == _queue.end)
        {
            bit = 0;
        }
        else
        {
        }
    }

private:
    LoggerBuffer   _buffer[N];
    std::bitset<N> _isAvilable[N];
    int            left[N];
    int            right[N];
    struct
    {
        int data[N];
        int start;
        int end;
    } _queue;
};

class AsyLogger : Noncopyable
{
public:
    AsyncLogging();
    void append(const char* data, size_t size);

private:
private:
    std::string                   _logName;
    int                           _flushInterval;
    std::unique_ptr<LoggerBuffer> writeBuffer;
};

// __PRETTY_FUNCTION__ is a more readable than __func__ in gcc

#define LOG_TRACE()

#define LOG_DEBUG() debugLogger;

#define LOG_INFO()

#define LOG_ERROR()
