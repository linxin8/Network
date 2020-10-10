#pragma once
#include "circle_queue.h"
#include "fixed_byte_buffer.h"
#include "thread.h"
#include "time.h"
#include <bitset>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

class LoggerBuffer : Noncopyable
{
public:
    LoggerBuffer()  = default;
    ~LoggerBuffer() = default;
    void clear()
    {
        _buffer.clear();
    }
    size_t getAvaliableSize()
    {
        return _buffer.availableSize();
    }
    template <typename T>
    LoggerBuffer& operator<<(T x)
    {
        _buffer.append(x);
        return *this;
    }

    void append(const char* data, size_t size)
    {
        _buffer.append(data, size);
    }

    const void* getRowdata() const
    {
        return _buffer.rowData();
    }

    // get size of buffer inserted
    size_t getSize() const
    {
        return _buffer.size();
    }

private:
    FixedByteBuffer<1000> _buffer;
};

template <size_t N>
class LogBufferPool : Noncopyable
{
public:
    // automatically give back buffer on destruction
    class BufferAgent
    {
    public:
        BufferAgent(LogBufferPool<N>* pool, LoggerBuffer* buffer) : _pool{pool}, _buffer{buffer} {}
        BufferAgent(BufferAgent&& agent) : _pool{agent._pool}, _buffer{agent._buffer}
        {
            agent._pool   = nullptr;
            agent._buffer = nullptr;
        }
        BufferAgent(const BufferAgent& agent) = delete;
        BufferAgent& operator                 =(BufferAgent&& agent)
        {
            _pool         = agent._pool;
            _buffer       = agent._buffer;
            agent._pool   = nullptr;
            agent._buffer = nullptr;
            return *this;
        }
        BufferAgent& operator=(const BufferAgent& agent) = delete;
        ~BufferAgent()
        {
            if (_pool != nullptr)
            {
                assert(_buffer != nullptr);
                _buffer->clear();
                _pool->giveBack(*_buffer);
            }
        };
        LoggerBuffer& getBuffer()
        {
            return *_buffer;
        }
        bool size() const
        {
            return _pool->getSize();
        }

    private:
        LogBufferPool* _pool;
        LoggerBuffer*  _buffer;
    };

    LogBufferPool()
    {
        // init avaliable buffer index
        for (int i = 0; i < N; i++)
        {
            _queue.push(i);
        }
    }
    // get avilable buffer
    BufferAgent getAvaliableAgent()
    {
        assert(!_queue.isEmpty());  // queue is empty, cannnot take any avaliable
        return {this, &_buffer[_queue.take()]};
    }

    bool isEmpty() const
    {
        return _queue.isEmpty();
    }

    size_t getSize() const
    {
        return _queue.getSize();
    }

private:
    // give back buffer, it will be avaliable
    void giveBack(const LoggerBuffer& buffer)
    {
        int index = &buffer - _buffer;
        assert(0 <= index && index < N);
        _queue.push(index);
    }

private:
    LoggerBuffer        _buffer[N];
    CircleQueue<int, N> _queue;
};

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
    void append(const char* data, size_t size);
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
    constexpr static size_t fixedSize = 1024;
    using LogBufferPool_t             = LogBufferPool<fixedSize>;
    std::string                                          _logName;
    int                                                  _flushInterval;
    LogBufferPool_t                                      _pool;
    CircleQueue<LogBufferPool_t::BufferAgent, fixedSize> _agentToWrited;
    // std::queue<LogBufferPool_t::BufferAgent> _agentToWrited;
    LogBufferPool_t::BufferAgent _currentAgent;
    std::mutex                   _mutex;
    // condition that data is ready to write
    std::condition_variable                    _condition;
    std::atomic_bool                           _isRunning;
    Thread                                     _thread;
    std::atomic_bool                           _isWaiting;
    std::vector<std::unique_ptr<LoggerBuffer>> _additionBuffer;
    std::atomic_bool                           _isUsingAdditionBuffer;
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
    Logger(const char* formatted_prefix_file_line, const char* function) : _buffer{}
    {
        static bool logStarted = false;
        if (!logStarted)
        {
            _logger.start();
            logStarted = true;
        }
        auto time = TimePoint::now();
        _buffer << time.hours() << ':' << time.minute() << ':' << time.second() << ' ' << formatted_prefix_file_line
                << function << ' ';
    }
    ~Logger()
    {
        assert(_buffer.getSize() < _buffer.getAvaliableSize());
        _buffer << '\n';
        _logger.append(static_cast<const char*>(_buffer.getRowdata()), _buffer.getSize());
    }

    template <typename T>
    Logger<true>& operator<<(const T& data)
    {
        _buffer << data << ' ';
        return *this;
    }

private:
    static inline AsyncLogger _logger;
    LoggerBuffer              _buffer;
};

template <>
class Logger<false>
{
public:
    constexpr Logger(const char*, const char*) {}
    template <typename T>
    Logger<false>& operator<<(const T& data)
    {
        return *this;
    }
};

// __PRETTY_FUNCTION__ is a more readable than __func__ in gcc

#define _LOG_FORMAT(PREFIX, FILE, LINE) PREFIX " " FILE " " LINE " "

#define _STRINGIZE_DETAIL(x) #x

#define _STRINGIZE(x) _STRINGIZE_DETAIL(x)

#define LOG_TRACE()                                                                                                    \
    Logger<LoggerLevel::trace >= logMinLevel>(_LOG_FORMAT("trace", __FILE__, _STRINGIZE(__LINE__)), __PRETTY_FUNCTION__)

#define LOG_DEBUG()                                                                                                    \
    Logger<LoggerLevel::debug >= logMinLevel>(_LOG_FORMAT("debug", __FILE__, _STRINGIZE(__LINE__)), __PRETTY_FUNCTION__)

#define LOG_INFO()                                                                                                     \
    Logger<LoggerLevel::info >= logMinLevel>(_LOG_FORMAT("info", __FILE__, _STRINGIZE(__LINE__)), __PRETTY_FUNCTION__)

#define LOG_ERROR()                                                                                                    \
    Logger<LoggerLevel::error >= logMinLevel>(_LOG_FORMAT("error", __FILE__, _STRINGIZE(__LINE__)), __PRETTY_FUNCTION__)
