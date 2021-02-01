#pragma once
#include "log_buffer.h"
#include "thread.h"
#include "time.h"
#include <cassert>
#include <condition_variable>
#include <exception>
#include <mutex>
#include <vector>

class AsyncLogger : Noncopyable
{
public:
    AsyncLogger();
    ~AsyncLogger()
    {
        if (_isRunning)
        {
            stop();
        }
    }

    // append log, if logger is stopped, do nothing
    void append(LogBuffer buffer);
    void start()
    {
        _isRunning = true;
        _thread.start();
    }

    // write remain data and stop log
    void stop()
    {
        _isRunning = false;
        _condition.notify_all();
        _thread.join();
    }

private:
    // thrad function that write data
    void writingThreadFunc();

private:
    std::string _logName;
    int         _flushInterval;
    std::mutex  _mutex;
    // condition that data is ready to write
    std::condition_variable _condition;
    std::atomic_bool        _isRunning;
    Thread                  _thread;
    std::atomic_bool        _isWaiting;
    std::vector<LogBuffer>  _writeVector;
};

enum class LoggerLevel
{
    none,
    trace,
    debug,
    info,
    error
};

constexpr LoggerLevel logMinLevel = LoggerLevel::debug;

template <bool>
class Logger;

template <>
class Logger<true>
{
public:
    Logger(const char* formatted_prefix_file_line,
           const char* function,
           const char* stackTrace = nullptr) :
        _buffer{LogBufferPool::getBuffer(64)}, _stackTrace{stackTrace}
    {
        static bool logStarted = false;
        if (!logStarted)
        {
            _logger.start();
            logStarted = true;
        }
        char str[256];
        auto time = TimePoint::now();
        int  n    = std::snprintf(str,
                              256,
                              "%d:%d:%d %s %s %s",
                              time.hours(),
                              time.minute(),
                              time.second(),
                              CurrentThread::getName().c_str(),
                              formatted_prefix_file_line,
                              function);
        _buffer.append(str, n);
    }
    ~Logger()
    {
        _buffer.append('\n');
        if (_stackTrace != nullptr)
        {
            _buffer.append(_stackTrace);
            _buffer.append('\n');
        }
        _logger.append(std::move(_buffer));
    }

    template <typename T>
    Logger<true>& operator<<(const T& data)
    {
        _buffer.append(' ');
        _buffer.append(data);
        return *this;
    }

private:
    static inline AsyncLogger _logger;
    LogBuffer                 _buffer;
    const char*               _stackTrace;
};

template <>
class Logger<false>
{
public:
    constexpr Logger(const char*, const char*) {}
    template <typename T>
    constexpr const Logger<false>& operator<<(const T& data) const
    {
        return *this;
    }
};

// __PRETTY_FUNCTION__ is a more readable than __func__ in gcc

#define _LOG_FORMAT(PREFIX, FILE, LINE) PREFIX " " FILE ":" LINE " "

#define _STRINGIZE_DETAIL(x) #x

#define _STRINGIZE(x) _STRINGIZE_DETAIL(x)

#define LOG_TRACE()                                                            \
    Logger<LoggerLevel::trace >= logMinLevel>(                                 \
        _LOG_FORMAT("trace", __FILE__, _STRINGIZE(__LINE__)),                  \
        __PRETTY_FUNCTION__)

#define LOG_DEBUG()                                                            \
    Logger<LoggerLevel::debug >= logMinLevel>(                                 \
        _LOG_FORMAT("debug", __FILE__, _STRINGIZE(__LINE__)),                  \
        __PRETTY_FUNCTION__)

#define LOG_INFO()                                                             \
    Logger<LoggerLevel::info >= logMinLevel>(                                  \
        _LOG_FORMAT("info", __FILE__, _STRINGIZE(__LINE__)),                   \
        __PRETTY_FUNCTION__)

#define LOG_ERROR()                                                            \
    Logger<LoggerLevel::error >= logMinLevel>(                                 \
        _LOG_FORMAT("error", __FILE__, _STRINGIZE(__LINE__)),                  \
        __PRETTY_FUNCTION__,                                                   \
        CurrentThread::getStackTrace().c_str())

inline void logAssertFailed()
{
    abort();
}

#define LOG_ASSERT(x)                                                          \
    {                                                                          \
        bool condition = static_cast<bool>(x);                                 \
        if (!condition)                                                        \
        {                                                                      \
            LOG_ERROR() << "assert failed \"" << #x << "\" :" << (x) << "\n"   \
                        << CurrentThread::getStackTrace();                     \
            logAssertFailed();                                                 \
        }                                                                      \
    }
