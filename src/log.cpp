#include "log.h"
#include "file.h"
#include <chrono>
#include <iostream>

AsyncLogger::AsyncLogger()
    : _logName{"log.txt"}, _flushInterval{100}, _pool{}, _agentToWrited{}, _currentAgent{_pool.getAvaliableAgent()},
      _condition{}, _isRunning{}, _thread{std::bind(&AsyncLogger::writingThreadFunc, this)}
{
}

void AsyncLogger::append(const char* data, size_t size)
{
    // if not running , just discard data
    if (!_isRunning)
    {
        return;
    }
    std::lock_guard<std::mutex> guard{_mutex};
    if (_pool.isEmpty())
    {
        fprintf(stderr, "pool is full, cannot append message %*s\n", static_cast<int>(size), data);
        return;
    }
    // printf("avaliable: %ld, require %ld\n", _currentAgent.getBuffer().getAvaliableSize(), size);
    // not enough space, save and replace current buffer
    if (_currentAgent.getBuffer().getAvaliableSize() < size)
    {
        // printf("new agent avaliable: %ld, require %ld, pool size %ld\n",
        // _currentAgent.getBuffer().getAvaliableSize(),
        //        size, _pool.getSize());
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
    std::queue<LogBufferPool<64>::BufferAgent> deleteQueue;

    // if not running, just write all remained buffer
    while (_isRunning || !agnetWritingQueue.empty() || _currentAgent.getBuffer().getSize() > 0)
    {
        assert(agnetWritingQueue.empty());  // assume queue is empty
        {
            std::unique_lock<std::mutex> lock{_mutex};
            while (!deleteQueue.empty())
            {
                deleteQueue.pop();
                //     static int id = 1;
                //     printf("pop %d agent size %ld pool size %ld\n", id++, agnetWritingQueue.size(), _pool.getSize());
            }
            _condition.wait_for(lock, std::chrono::milliseconds{_flushInterval});
            if (_agentToWrited.empty())
            {  // no full buffered data, just wirte current buffer
                agnetWritingQueue.push(std::move(_currentAgent));
                _currentAgent = std::move(_pool.getAvaliableAgent());
            }
            else
            {
                agnetWritingQueue.swap(_agentToWrited);
            }
        }
        while (!agnetWritingQueue.empty())  // write all data
        {
            auto& buffer = agnetWritingQueue.front().getBuffer();
            file.append(static_cast<const char*>(buffer.getRowdata()), buffer.getSize());
            deleteQueue.push(std::move(agnetWritingQueue.front()));
            agnetWritingQueue.pop();
        }
        file.flush();
    }
}