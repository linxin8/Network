#pragma once
#include "circle_queue.h"
#include <bit>
#include <cassert>
#include <cstring>
#include <memory>
#include <type_traits>

// not thread safe
class TcpBuffer
{
    static_assert(std::endian::native ==
                  std::endian::little);  // assure local is little endian
public:
    TcpBuffer() :
        _smallBuffer{std::make_unique<CircleQueue<char, 8 * 1024>>()},
        _largeBuffer{}  // lazy alloc
    {
    }

    // get the size of unsent data
    size_t getAvailableSize() const
    {
        if (_smallBuffer)
        {
            return _smallBuffer->getAvailableSize();
        }
        else
        {
            return _largeBuffer->getAvailableSize();
        }
    }

    template <typename T>
    T peek()
    {
        static_assert(std::is_pod_v<T>);
        assert(getSize() >= sizeof(T));
        char buffer[sizeof(T)];
        if (_smallBuffer)
        {
            if (_smallBuffer->getContinousSize() >= sizeof(T))
            {
                return reinterpret_cast<T&>(*_smallBuffer->getFirstAddress());
            }
            else
            {
                memcpy(buffer,
                       _smallBuffer->getFirstAddress(),
                       _smallBuffer->getContinousSize());
                memcpy(buffer + _smallBuffer->getContinousSize(),
                       _smallBuffer->getRowData(),
                       sizeof(T) - _smallBuffer->getContinousSize());
                return reinterpret_cast<T&>(buffer);
            }
        }
        else
        {
            if (_largeBuffer->getContinousSize() >= sizeof(T))
            {
                return reinterpret_cast<T&>(*_largeBuffer->getFirstAddress());
            }
            else
            {
                memcpy(buffer,
                       _largeBuffer->getFirstAddress(),
                       _largeBuffer->getContinousSize());
                memcpy(buffer + _largeBuffer->getContinousSize(),
                       _largeBuffer->getRowData(),
                       sizeof(T) - _largeBuffer->getContinousSize());
                return reinterpret_cast<T&>(buffer);
            }
        }
    }

    template <typename T>
    T take()
    {
        static_assert(std::is_pod_v<T>);
        assert(getSize() >= sizeof(T));
        char buffer[sizeof(T)];
        if (_smallBuffer)
        {
            if (_smallBuffer->getContinousSize() >= sizeof(T))
            {
                memcpy(buffer, _smallBuffer->getFirstAddress(), sizeof(T));
            }
            else
            {
                memcpy(buffer,
                       _smallBuffer->getFirstAddress(),
                       _smallBuffer->getContinousSize());
                memcpy(buffer + _smallBuffer->getContinousSize(),
                       _smallBuffer->getRowData(),
                       sizeof(T) - _smallBuffer->getContinousSize());
            }
            _smallBuffer->pop(sizeof(T));
            return reinterpret_cast<T&>(buffer);
        }
        else
        {
            if (_largeBuffer->getContinousSize() >= sizeof(T))
            {
                memcpy(buffer, _largeBuffer->getFirstAddress(), sizeof(T));
            }
            else
            {
                memcpy(buffer,
                       _largeBuffer->getFirstAddress(),
                       _largeBuffer->getContinousSize());
                memcpy(buffer + _largeBuffer->getContinousSize(),
                       _largeBuffer->getRowData(),
                       sizeof(T) - _largeBuffer->getContinousSize());
            }
            _largeBuffer->pop(sizeof(T));
            return reinterpret_cast<T&>(buffer);
        }
    }

    void pop(int popSize)
    {
        if (_smallBuffer)
        {
            _smallBuffer->pop(popSize);
        }
        else
        {
            _largeBuffer->pop(popSize);
        }
    }

    size_t getSize() const
    {
        if (_smallBuffer)
        {
            return _smallBuffer->getSize();
        }
        return _largeBuffer->getSize();
    }

    // append data stream with maximun size
    // reutrn actually size pushed
    template <typename T>
    bool append(const T& x)
    {
        static_assert(std::is_pod_v<T>);
        static_assert(!std::is_pointer_v<T>);
        if (_smallBuffer)
        {
            if (_smallBuffer->getAvailableSize() < sizeof(T))
            {
                return false;
            }
            _smallBuffer->push(x);
        }
        else
        {
            if (_largeBuffer->getAvailableSize() < sizeof(T))
            {
                return false;
            }
            _largeBuffer->push(x);
        }
        return true;
    }

    // append data stream with maximun size
    // reutrn actually size pushed
    size_t append(const void* data, size_t maxSize)
    {
        if (_smallBuffer)
        {
            assert(!_largeBuffer);
            if (_smallBuffer->getCapacity() < maxSize)
            {
                // use large buffer instead
                _largeBuffer =
                    std::make_unique<CircleQueue<char, 2 * 1024 * 1024>>();
                _largeBuffer->push(_smallBuffer->getFirstAddress(),
                                   _smallBuffer->getContinousSize());
                _smallBuffer->popContinouData();
                if (_smallBuffer->getContinousSize() > 0)
                {
                    _largeBuffer->push(_smallBuffer->getFirstAddress(),
                                       _smallBuffer->getContinousSize());
                }
                _smallBuffer.reset();
                return _largeBuffer->push(data, maxSize);
            }
            else
            {
                return _smallBuffer->push(data, maxSize);
            }
        }
        else
        {
            assert(!_smallBuffer);
            return _largeBuffer->push(data, maxSize);
        }
    }

private:
    std::unique_ptr<CircleQueue<char, 8 * 1024>>        _smallBuffer;  // 8k
    std::unique_ptr<CircleQueue<char, 2 * 1024 * 1024>> _largeBuffer;  // 2M
};