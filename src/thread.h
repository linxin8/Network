#pragma once
#include "count_down_latch.h"
#include "type.h"
#include <atomic>
#include <functional>

namespace CurrentThread
{
    const std::string& getName();
    const std::string& getTidString();
    pid_t              getTid();
    bool               isMainThread();
    void               sleep(int64_t microseconds);
    void               setName(std::string name);
    // @demangle: whether use demangle to deformat function name
    std::string getStackTrace(bool demangle = true);
}  // namespace CurrentThread

class Thread : Noncopyable
{
public:
    explicit Thread(std::function<void()> threadMain);
    explicit Thread(std::function<void()> threadMain, std::string name);
    ~Thread();
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
    bool                        _isStarted;
    bool                        _isJoined;
    std::function<void()>       _threadMain;
    static std::atomic_uint32_t _numCreated;
    std::string                 _name;
    pthread_t                   _pthreadID;
    pid_t                       _tid;
    CountDownLatch              _latch;
};