#pragma once
#include <bitset>
#include <cassert>
#include <charconv>
#include <cstring>
#include <mutex>
#include <string>

class LogBufferPool
{
    struct LogPool
    {
        static const constexpr size_t maxPoolSize = 4096;
        std::string                   buffer[maxPoolSize];
        std::bitset<maxPoolSize>      statusBits;
        const size_t                  poolSize;
        const size_t                  bufferSize;
        std::mutex                    mutex;
        LogPool(size_t poolSize, size_t bufferSize) :
            poolSize{poolSize}, bufferSize{bufferSize}
        {
            assert(poolSize <= maxPoolSize);
            statusBits.reset();
            for (int i = 0; i < poolSize; i++)
            {
                statusBits[i] = true;
                buffer[i].resize(bufferSize);
            }
        }
    };

public:
    class LogBuffer
    {
    public:
        explicit LogBuffer(size_t size) :
            _pool{nullptr},
            _bufferIndex{0},
            _writeIndex{0},
            _isOwned{false},
            _largeLogData{new std::string(size, '\0')},
            _capcity{size}
        {
        }
        LogBuffer(LogPool* pool, size_t bufferIndex) :
            _pool{pool},
            _bufferIndex{bufferIndex},
            _writeIndex{0},
            _isOwned{true},
            _largeLogData{nullptr},
            _capcity{pool->buffer[bufferIndex].size()}
        {
            // already locked
            assert(!pool->mutex.try_lock());
            pool->statusBits[bufferIndex] = false;
        }
        LogBuffer(LogBuffer&& r) :
            _pool{r._pool},
            _bufferIndex{r._bufferIndex},
            _writeIndex{r._writeIndex},
            _isOwned{r._isOwned},
            _largeLogData{r._largeLogData},
            _capcity{r._capcity}
        {
            r._isOwned      = false;
            r._largeLogData = nullptr;
            r._bufferIndex  = 0;
            r._pool         = nullptr;
            r._writeIndex   = 0;
            r._capcity      = 0;
        }
        ~LogBuffer()
        {
            release();
        }

        size_t size() const
        {
            return _writeIndex;
        }
        const void* rawData() const
        {
            if (_isOwned)
            {
                return _pool->buffer[_bufferIndex].c_str();
            }
            assert(_largeLogData);
            return _largeLogData->c_str();
        }
        void* rawData()
        {
            if (_isOwned)
            {
                return _pool->buffer[_bufferIndex].data();
            }
            assert(_largeLogData);
            return _largeLogData->data();
        }
        void append(const void* data, size_t size)
        {
            assureCapable(size);
            std::memcpy(
                static_cast<char*>(rawData()) + _writeIndex, data, size);
            _writeIndex += size;
        }
        void append(const char* str)
        {
            append(static_cast<const void*>(str), strlen(str));
        }

        void append(char* str)
        {
            append(static_cast<void*>(str), strlen(str));
        }

        template <typename T>
        void append(const T& x)
        {
            char buffer[32];  // string length can not excesses 32
            std::to_chars_result&& result =
                std::to_chars(buffer, buffer + sizeof(buffer), x);
            append(buffer, result.ptr - buffer);
        }

        void append(const std::string& x)
        {
            append(x.c_str(), x.length());
        }

        void append(bool x)
        {
            x ? append("true", 4) : append("false", 5);
        }

        template <typename T>
        void append(T* x)
        {
            append(static_cast<const void*>(x));
        }

        // append pointer, start by 0x prefix
        void append(const void* x)
        {
            constexpr int          size = sizeof(void*) + 2;
            char                   buffer[32]{"0x"};
            std::to_chars_result&& result =
                std::to_chars(buffer + 2,
                              buffer + sizeof(buffer),
                              reinterpret_cast<size_t>(x),
                              16);
            append(buffer, result.ptr - buffer);
        }

        void append(double x)
        {
            char str[32];  // double string length can not excesses 32
            int  size =
                sprintf(str, "%lf", x);  // to do, wait for implementation of
                                         // std::to_chars on double argument
            append(str, size);
        }

        void append(float x)
        {
            char str[32];  // double string length can not excesses 32
            int  size =
                sprintf(str, "%f", x);  // to do, wait for implementation of
                                        // std::to_chars on float argument
            append(str, size);
        }

        void append(char x)
        {
            append(&x, 1);
        }

    private:
        void release()
        {
            if (_isOwned)
            {
                _pool->mutex.lock();
                _pool->statusBits[_bufferIndex] = true;
                _pool->mutex.unlock();
                _isOwned = false;
            }
            if (_largeLogData)
            {
                delete _largeLogData;
                _largeLogData = nullptr;
            }
        }
        LogBuffer& operator=(LogBuffer&& r)
        {
            release();
            _pool           = r._pool;
            _bufferIndex    = r._bufferIndex;
            _writeIndex     = r._writeIndex;
            _isOwned        = r._isOwned;
            _largeLogData   = r._largeLogData;
            _capcity        = r._capcity;
            r._isOwned      = false;
            r._largeLogData = nullptr;
            r._bufferIndex  = 0;
            r._pool         = nullptr;
            r._writeIndex   = 0;
            r._capcity      = 0;
            return *this;
        }
        void expand(size_t newSize)
        {
            size_t ns = _capcity;
            if (ns >= newSize)
            {
                return;
            }
            while (ns < newSize)
            {
                ns *= 2;
            }
            auto newBuffer = LogBufferPool::getBuffer(newSize);
            newBuffer.append(rawData(), size());
            release();
            *this = std::move(newBuffer);
        };

        void assureCapable(size_t writeSize)
        {
            auto nextSize = _writeIndex + writeSize;
            expand(nextSize);
        }

    private:
        LogPool* _pool;
        size_t   _bufferIndex;
        size_t   _writeIndex;
        bool     _isOwned;  // whether this object has owned a buffer in pool
        std::string* _largeLogData;
        size_t _capcity;  // size of storage that can be held without expand
    };

public:
    static LogBuffer getBuffer(size_t size = 64)
    {
        for (auto& p : _poolSet)
        {
            if (p.bufferSize >= size)
            {
                std::lock_guard<std::mutex> guard{p.mutex};
                auto                        index = p.statusBits._Find_first();
                if (index < p.poolSize)
                {
                    return {&p, index};
                }
            }
        }
        return LogBuffer{size};
    }

private:
    static inline LogPool _poolSet[6]{
        {64, 128},
        {64, 256},
        {64, 512},
        {64, 1024},
        {64, 2048},
        {64, 4096},
    };
};

using LogBuffer = LogBufferPool::LogBuffer;