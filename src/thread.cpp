#include "thread.h"
#include "exception.h"
#include <cassert>
#include <chrono>
#include <ctime>
#include <cxxabi.h>
#include <execinfo.h>
#include <linux/unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

std::atomic_uint32_t Thread::_numCreated{};

namespace CurrentThread
{
    thread_local pid_t _tid = [] {
        return static_cast<pid_t>(syscall(SYS_gettid));
    }();
    thread_local std::string _name = [] {
        return std::string("Thread") + std::to_string(_tid);
    }();
    thread_local std::string _tidString = [] { return std::to_string(_tid); }();

    void sleep(int64_t microseconds)
    {
        timespec      ts;
        constexpr int microsecondsPerSecond = 1000 * 1000;
        ts.tv_sec = static_cast<time_t>(microseconds / microsecondsPerSecond);
        ts.tv_nsec =
            static_cast<long>(microseconds % microsecondsPerSecond * 1000);
        nanosleep(&ts, NULL);
    }
    std::string getStackTrace(bool demangle)
    {
        std::string result;
        const int   max_frame = 200;
        void*       frame[max_frame]{};
        int         nptrs   = backtrace(frame, max_frame);
        char**      strings = backtrace_symbols(frame, nptrs);
        if (strings != nullptr)
        {
            size_t len = 256;
            char*  demangled =
                demangle ? static_cast<char*>(::malloc(len)) : nullptr;
            for (int i = 1; i < nptrs;
                 ++i)  // skipping the 0-th, which is this function
            {
                if (demangle)
                {
                    // https://panthema.net/2008/0901-stacktrace-demangled/
                    // bin/exception_test(_ZN3Bar4testEv+0x79) [0x401909]
                    char* left_par = nullptr;
                    char* plus     = nullptr;
                    for (char* p = strings[i]; *p; ++p)
                    {
                        if (*p == '(')
                        {
                            left_par = p;
                        }
                        else if (*p == '+')
                        {
                            plus = p;
                        }
                    }

                    if (left_par && plus)
                    {
                        *plus        = '\0';
                        int   status = 0;
                        char* ret    = abi::__cxa_demangle(
                            left_par + 1, demangled, &len, &status);
                        *plus = '+';
                        if (status == 0)
                        {
                            demangled = ret;  // ret could be realloc()
                            result.append(strings[i], left_par + 1);
                            result.append(demangled);
                            result.append(plus);
                            result.push_back('\n');
                            continue;
                        }
                    }
                }
                // Fallback to mangled names
                result.append(strings[i]);
                result.push_back('\n');
            }
            free(demangled);
            free(strings);
        }
        return std::move(result);
    }
};  // namespace CurrentThread

Thread::Thread(std::function<void()> threadMain, std::string name) :
    _threadMain{std::move(threadMain)},
    _isStarted{},
    _isJoined{},
    _name{std::move(name)},
    _pthreadID{},
    _tid{},
    _latch{1}
{
    ++_numCreated;
}

Thread::Thread(std::function<void()> threadMain) :
    Thread{std::move(threadMain),
           std::string("Thread") + std::to_string(_numCreated + 1)}
{
}

struct PThreadMainArgument
{
    std::function<void()> _mainFunction;
    std::string           _name;
    pid_t*                _tid;
    CountDownLatch*       _latch;
    PThreadMainArgument(std::function<void()> mainFunction,
                        std::string           name,
                        pid_t*                tid,
                        CountDownLatch*       latch) :
        _mainFunction{std::move(mainFunction)},
        _name{std::move(name)},
        _tid{tid},
        _latch{latch}
    {
    }
};

void* phtreadMain(void* data)
{
    PThreadMainArgument* argument = static_cast<PThreadMainArgument*>(data);
    *argument->_tid               = CurrentThread::getTid();
    argument->_tid                = nullptr;
    argument->_latch->countDown();
    argument->_latch = nullptr;

    if (!argument->_name.empty())
    {
        CurrentThread::setName(argument->_name);
    }
    prctl(PR_SET_NAME, CurrentThread::getName().c_str());
    try
    {
        argument->_mainFunction();
        CurrentThread::setName("finished");
    }
    catch (const Exception& ex)
    {
        CurrentThread::setName("crashed");
        fprintf(stderr,
                "exception caught in Thread %s\n",
                CurrentThread::getName().c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
    catch (const std::exception& ex)
    {
        CurrentThread::setName("crashed");
        fprintf(stderr,
                "exception caught in Thread %s\n",
                CurrentThread::getName().c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        CurrentThread::setName("crashed");
        fprintf(stderr,
                "unknown exception caught in Thread %s\n",
                CurrentThread::getName().c_str());
        throw;  // rethrow
    }
    return nullptr;
}

// create thread, return whether success
bool pthreadStart(std::function<void()> mainFunction,
                  std::string           name,
                  pid_t*                tid,
                  pthread_t*            thread)
{
    CountDownLatch       latch{1};
    PThreadMainArgument* arg = new PThreadMainArgument{
        std::move(mainFunction), std::move(name), tid, &latch};
    if (pthread_create(thread, NULL, &phtreadMain, arg))
    {
        fprintf(stderr, "create pthread failed\n");
        delete arg;
        return false;
    }
    else
    {
        latch.wait();
        assert(*tid > 0);
    }
    return true;
}

void Thread::start()
{
    assert(!_isStarted);
    _isStarted = pthreadStart(_threadMain, _name, &_tid, &_pthreadID);
}

int Thread::join()
{
    assert(_isStarted);
    assert(!_isJoined);
    _isJoined = true;
    return pthread_join(_pthreadID, NULL);
}
Thread::~Thread()
{
    if (_isStarted && !_isJoined)
    {
        pthread_detach(_pthreadID);
    }
}
