#pragma once
#include "count_down_latch.h"
#include "type.h"
#include <atomic>
#include <functional>
#include <unistd.h>

namespace CurrentThread
{
    extern thread_local pid_t       _tid;
    extern thread_local std::string _name;
    extern thread_local std::string _tidString;

    void                      setName(std::string name);
    inline const std::string& getName()
    {
        return _name;
    }
    inline const std::string& getTidString()
    {
        return _tidString;
    }
    inline pid_t getTid()
    {
        return _tid;
    }
    inline bool isMainThread()
    {
        return _tid == getpid();
    }
    void sleep(int64_t microseconds);
    void sleeps(int64_t seconds);
    // @demangle: whether use demangle to deformat function name
    std::string getStackTrace(int maxFrame = 10, bool demangle = true);
}  // namespace CurrentThread

class Thread : Noncopyable
{
public:
    explicit Thread(std::function<void()> threadMain);
    explicit Thread(std::function<void()> threadMain, std::string name);
    ~Thread();
    // return only after thread has run
    void               start();
    int                join();
    void               detach();
    const std::string& getName() const
    {
        return _name;
    }
    pid_t getTid() const
    {
        return _tid;
    }
    bool isStarted() const
    {
        return _isStarted;
    }
    // get number of thread that have been created
    static int getNumberCreated()
    {
        return _numCreated;
    }

private:
    std::atomic_bool            _isStarted;
    std::atomic_bool            _isJoined;
    std::function<void()>       _threadMain;
    static std::atomic_uint32_t _numCreated;
    std::string                 _name;
    pthread_t                   _pthreadID;
    pid_t                       _tid;
    CountDownLatch              _latch;
};