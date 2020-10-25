#pragma once
#include "circle_queue.h"
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
    TcpBuffer() : _queue{_smallBufferSize} {}

    // get the size of data can be appended
    size_t getAvailableSize() const
    {
        return _largeBufferSize - _queue.getSize();
    }

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

    // get the address of first element
    // may get invalid after any element removed or inserted
    char* getFirstAddress()
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
        assert(getSize() >= sizeof(T));
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
        assert(getSize() >= sizeof(T));
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
        assert(getAvailableSize() >= sizeof(T));
        append(&x, sizeof(T));
    }

    // append data stream with maximun size
    // may only append partial data
    // reutrn actually size appended
    size_t append(const void* data, size_t maxSize)
    {
        if (_queue.getAvailableSize() < maxSize)
        {
            if (_queue.getCapacity() == _smallBufferSize)
            {
                // use large buffer instead
                _queue.resetCapcity(_largeBufferSize);
            }
        }
        return _queue.push(data, maxSize);
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
        if (_queue.isFull() && isSmallBuffer())
        {
            // use large buffer instead
            _queue.resetCapcity(_largeBufferSize);
        }
    }

private:
    bool isSmallBuffer() const
    {
        return _queue.getSize() == _smallBufferSize;
    }

private:
    CircleByteStreamQueue _queue;
};