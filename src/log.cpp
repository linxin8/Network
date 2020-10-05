#include "log.h"
#include "file.h"
#include <chrono>
#include <iostream>

AsyncLogger::AsyncLogger()
    : _logName{"log.txt"}, _flushInterval{3}, _pool{}, _agentToWrited{}, _currentAgent{_pool.getAvaliableAgent()},
      _condition{}, _isRunning{}, _thread{std::bind(&AsyncLogger::writingThreadFunc, this)}
{
}

void AsyncLogger::append(const char* data, size_t size)
{
    std::lock_guard<std::mutex> guard{_mutex};
    if (_pool.isFull())
    {
        fprintf(stderr, "pool is full, cannot append message %*s\n", static_cast<int>(size), data);
        return;
    }
    // not enough space, save and replace current buffer
    if (_currentAgent.getBuffer().getAvaliableSize() < size)
    {
        _agentToWrited.push(std::move(_currentAgent));
        _currentAgent = std::move(_pool.getAvaliableAgent());
    }
    auto& buffer = _currentAgent.getBuffer();
    buffer.append(data, size);
    _condition.notify_one();  // only one thread is writing data
}

void AsyncLogger::writingThreadFunc()
{
    WriteOnlyFile                              file{_logName.c_str()};
    std::queue<LogBufferPool<64>::BufferAgent> agnetWritingQueue;
    while (_isRunning)
    {
        assert(agnetWritingQueue.empty());  // assume queue is empty
        {
            std::unique_lock<std::mutex> lock{_mutex};
            _condition.wait_for(lock, std::chrono::seconds{_flushInterval});
            agnetWritingQueue.swap(_agentToWrited);
        }
        while (!agnetWritingQueue.empty())  // write all data
        {
            auto& buffer = agnetWritingQueue.front().getBuffer();
            file.append(static_cast<const char*>(buffer.getRowdata()), buffer.getSize());
            agnetWritingQueue.pop();
        }
        file.flush();
    }
}