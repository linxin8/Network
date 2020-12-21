#pragma once
#include "circle_queue.h"
#include "log.h"
#include <bit>
#include <cassert>
#include <cstring>
#include <memory>
#include <type_traits>

// not thread safe
// circle tcp buffer, data are stored by up to 2 continous spaces
class TcpBuffer
{
    static_assert(std::endian::native ==
                  std::endian::little);  // assure local is little endian
private:
    static inline constexpr size_t _smallBufferSize = 8 * 1024;         // 8k
    static inline constexpr size_t _largeBufferSize = 2 * 1024 * 1024;  // 2M
public:
    TcpBuffer(size_t size = _smallBufferSize) : _queue{size} {}

    // return continous size of data can be read from first element
    size_t getContinousSize() const
    {
        return _queue.getContinousSize();
    }

    // get the address of first element
    // may get invalid after any element removed or inserted
    const char* getFirstAddress() const
    {
        return _queue.getFirstAddress();
    }

    // remove continous size of data can be read from first element
    void popContinouData()
    {
        _queue.popContinouData();
    }

    // assume size of data is enougth
    // must check size before call
    template <typename T>
    T peek()
    {
        static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
        LOG_ASSERT(getSize() >= sizeof(T));
        if (_queue.getContinousSize() >= sizeof(T))
        {
            return reinterpret_cast<T&>(*_queue.getFirstAddress());
        }
        else
        {
            char buffer[sizeof(T)];
            auto csize = _queue.getContinousSize();
            memcpy(buffer, _queue.getFirstAddress(), csize);
            memcpy(buffer + csize, _queue.getRowData(), sizeof(T) - csize);
            return reinterpret_cast<T&>(buffer);
        }
    }

    // assume size of data is enougth
    // must check size before call
    template <typename T>
    T take()
    {
        static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
        LOG_ASSERT(getSize() >= sizeof(T));
        char buffer[sizeof(T)];
        if (_queue.getContinousSize() >= sizeof(T))
        {
            memcpy(buffer, _queue.getFirstAddress(), sizeof(T));
        }
        else
        {
            auto csize = _queue.getContinousSize();
            memcpy(buffer, _queue.getFirstAddress(), csize);
            memcpy(buffer + csize, _queue.getRowData(), sizeof(T) - csize);
        }
        _queue.popN(sizeof(T));
        return reinterpret_cast<T&>(buffer);
    }

    void popN(int popSize)
    {
        _queue.popN(popSize);
    }

    // return size of element stored
    size_t getSize() const
    {
        return _queue.getSize();
    }

    // assume size is enougth
    // must check avaliable size before call
    template <typename T>
    void append(const T& x)
    {
        static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
        static_assert(!std::is_pointer_v<T>);
        append(&x, sizeof(T));
    }

    void append(const void* data, size_t size)
    {
        _queue.push(data, size);
    }

    // {wirting address, max size}
    // used for direct wirte for avoid copying buffer data
    // must call endDirectWrite once writing is finished
    std::pair<char*, size_t> beginDirectWrite()
    {
        return _queue.beginDirectWrite();
    }

    // used for direct wirte for avoid copying buffer data
    // must be called after beginDirectWrite
    // writtenSize is size of data that have been writed
    void endDirectWrite(size_t writtenSize)
    {
        _queue.endDirectWrite(writtenSize);
    }

private:
    DynamicCircleByteStreamQueue _queue;
};