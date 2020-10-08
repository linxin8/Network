#include "log.h"
#include "file.h"
#include <chrono>
#include <iostream>

AsyncLogger::AsyncLogger()
    : _logName{"log.txt"}, _flushInterval{10}, _pool{}, _agentToWrited{}, _currentAgent{_pool.getAvaliableAgent()},
      _condition{}, _isRunning{}, _thread{std::bind(&AsyncLogger::writingThreadFunc, this)}, _isWaiting{},
      _additionBuffer{}, _isUsingAdditionBuffer{}
{
}

void AsyncLogger::append(const char* data, size_t size)
{
    // if not running , just discard data
    if (!_isRunning)
    {
        return;
    }
    assert(size < fixedSize);  // buffer max size is fixedSize;
    {
        std::lock_guard<std::mutex> guard{_mutex};
        if (!_isUsingAdditionBuffer)
        {
            if (_currentAgent.getBuffer().getAvaliableSize() >= size)
            {
                auto& buffer = _currentAgent.getBuffer();
                buffer.append(data, size);
            }
            else if (!_pool.isEmpty())
            {
                _agentToWrited.push(std::move(_currentAgent));
                _currentAgent = std::move(_pool.getAvaliableAgent());
                auto& buffer  = _currentAgent.getBuffer();
                assert(buffer.getSize() == 0);
                assert(buffer.getAvaliableSize() > 0);
                buffer.append(data, size);
            }
            else
            {
                _isUsingAdditionBuffer = true;  //  append data to addition buffer
                if (_additionBuffer.empty() || _additionBuffer.back()->getAvaliableSize() < size)
                {
                    _additionBuffer.push_back(std::make_unique<LoggerBuffer>());
                }
                _additionBuffer.back()->append(data, size);
            }
        }
        if (_isWaiting)
        {
            _condition.notify_one();  // only one thread is writing data
            _isWaiting = false;
        }
    }
}

void AsyncLogger::writingThreadFunc()
{
    WriteOnlyFile                                        file{_logName.c_str()};
    CircleQueue<LogBufferPool_t::BufferAgent, fixedSize> agnetWritingQueue;
    CircleQueue<LogBufferPool_t::BufferAgent, fixedSize> deleteQueue;
    std::vector<std::unique_ptr<LoggerBuffer>>           additionBuffer;
    // if not running, discard all data
    while (_isRunning)
    {
        assert(agnetWritingQueue.isEmpty());  // assume queue is empty
        {
            std::unique_lock<std::mutex> lock{_mutex};
            deleteQueue.clear();
            if (_isUsingAdditionBuffer)
            {
                additionBuffer.swap(_additionBuffer);
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
                _isWaiting = true;
                _condition.wait_for(lock, std::chrono::milliseconds{_flushInterval});
            }
        }
        while (!agnetWritingQueue.isEmpty())  // write all data
        {
            auto& buffer = agnetWritingQueue.getFront().getBuffer();
            file.append(static_cast<const char*>(buffer.getRowdata()), buffer.getSize());
            deleteQueue.push(std::move(agnetWritingQueue.take()));
        }
        if (_isUsingAdditionBuffer)
        {
            for (auto& buffer : additionBuffer)
            {
                file.append(static_cast<const char*>(buffer->getRowdata()), buffer->getSize());
            }
            additionBuffer.clear();
            _isUsingAdditionBuffer = false;
        }
        // file.flush();
    }
}