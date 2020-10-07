#include "log.h"
#include "file.h"
#include <chrono>
#include <iostream>

AsyncLogger::AsyncLogger()
    : _logName{"log.txt"}, _flushInterval{100}, _pool{}, _agentToWrited{}, _currentAgent{_pool.getAvaliableAgent()},
      _condition{}, _isRunning{}, _thread{std::bind(&AsyncLogger::writingThreadFunc, this)}, _isWating{}
{
}

void AsyncLogger::append(const char* data, size_t size)
{
    // if not running , just discard data
    if (!_isRunning)
    {
        return;
    }
    bool error = false;
    {
        std::lock_guard<std::mutex> guard{_mutex};
        if (_currentAgent.getBuffer().getAvaliableSize() >= size)
        {
            auto& buffer = _currentAgent.getBuffer();
            buffer.append(data, size);
            _condition.notify_one();  // only one thread is writing data
        }
        else if (!_pool.isEmpty())
        {
            _agentToWrited.push(std::move(_currentAgent));
            _currentAgent = std::move(_pool.getAvaliableAgent());
            auto& buffer  = _currentAgent.getBuffer();
            buffer.append(data, size);
            if (_isWating)
            {
                _condition.notify_one();  // only one thread is writing data
            }
        }
        else
        {  // cannot append data
            error = true;
        }
    }
    if (error)
    {
        // fprintf(stderr, "log pool is full, discard message: %*s\n", static_cast<int>(size), data);
    }
}

void AsyncLogger::writingThreadFunc()
{
    WriteOnlyFile                                        file{_logName.c_str()};
    CircleQueue<LogBufferPool_t::BufferAgent, fixedSize> agnetWritingQueue;
    CircleQueue<LogBufferPool_t::BufferAgent, fixedSize> deleteQueue;

    // if not running, discard all data
    while (_isRunning)
    {
        assert(agnetWritingQueue.isEmpty());  // assume queue is empty
        {
            std::unique_lock<std::mutex> lock{_mutex};
            while (!deleteQueue.isEmpty())
            {
                deleteQueue.pop();
                // static int id = 1;
                // printf("pop %d deleteQueue\n", id++);
            }
            if (!_agentToWrited.isEmpty())
            {
                agnetWritingQueue.swap(_agentToWrited);
            }
            else if (_currentAgent.getBuffer().getSize() > 0)
            {  // no full buffered data, just wirte current buffer
                agnetWritingQueue.push(std::move(_currentAgent));
                _currentAgent = std::move(_pool.getAvaliableAgent());
            }
            else
            {  // no data, just wait
                _isWating = true;
                _condition.wait_for(lock, std::chrono::milliseconds{_flushInterval});
                _isWating = false;
            }
        }
        while (!agnetWritingQueue.isEmpty())  // write all data
        {
            auto& buffer = agnetWritingQueue.getFront().getBuffer();
            file.append(static_cast<const char*>(buffer.getRowdata()), buffer.getSize());
            deleteQueue.push(std::move(agnetWritingQueue.take()));
        }
        file.flush();
    }
}