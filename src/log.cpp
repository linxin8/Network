#include "log.h"
#include "file.h"
#include <chrono>
#include <iostream>

AsyncLogger::AsyncLogger() :
    _logName{"log.txt"},
    _flushInterval{10},
    _condition{},
    _isRunning{},
    _thread{std::bind(&AsyncLogger::writingThreadFunc, this), "log"},
    _isWaiting{}
{
}

void AsyncLogger::append(LogBuffer buffer)
{
    {
        // // or just print to stdout
        std::printf("%.*s",
                    static_cast<int>(buffer.size()),
                    static_cast<char*>(buffer.rawData()));
    }
    // if not running , just discard data
    if (!_isRunning)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> guard{_mutex};
        _writeVector.push_back(std::move(buffer));
        if (_isWaiting)
        {
            _condition.notify_one();  // only one thread is writing data
            _isWaiting = false;
        }
    }
}

void AsyncLogger::writingThreadFunc()
{
    WriteOnlyFile          file{_logName.c_str()};
    std::vector<LogBuffer> writeVector;
    // if not running, discard all data
    while (_isRunning)
    {
        assert(writeVector.empty());  // assume queue is empty
        {
            std::unique_lock<std::mutex> lock{_mutex};
            if (!_writeVector.empty())
            {
                writeVector.swap(_writeVector);
            }
            else
            {  // no data, just wait
                _isWaiting = true;
                _condition.wait_for(lock,
                                    std::chrono::milliseconds{_flushInterval});
            }
        }
        for (auto& buffer : writeVector)
        {
            file.append(static_cast<const char*>(buffer.rawData()),
                        buffer.size());
        }
        writeVector.clear();
        // file.flush();
    }
}