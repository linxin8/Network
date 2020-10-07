#pragma once
#include "circle_queue.h"
#include "fixed_byte_buffer.h"
#include "thread.h"
#include <bitset>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

enum class LoggerLevel
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
        stop();
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
    bool                                       _isRunning;
    Thread                                     _thread;
    std::atomic_bool                           _isWating;
    std::vector<std::unique_ptr<LoggerBuffer>> _additionBuffer;
    std::atomic_bool                           _isUsingAdditionBuffer;
};

// __PRETTY_FUNCTION__ is a more readable than __func__ in gcc

#define LOG_TRACE()

#define LOG_DEBUG() debugLogger;

#define LOG_INFO()

#define LOG_ERROR()
