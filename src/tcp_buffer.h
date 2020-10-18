#pragma once
#include <bit>
#include <cassert>
#include <cstring>
#include <type_traits>
#include <vector>

class TcpBuffer
{
    static_assert(std::endian::native ==
                  std::endian::little);  // assure local is little endian
public:
    size_t getSize() const
    {
        return _buffer.size();
    }

    size_t getReadIndex() const
    {
        return _readIndex;
    }

    void seekReadBegin()
    {
        _readIndex = 0;
    }

    void seekReadEnd()
    {
        _readIndex = _buffer.size();
    }

    // reset read index to (current index + offert)
    void seekReadOffset(int offset)
    {
        assert(_readIndex + offset >= 0);
        assert(_readIndex + offset <= _buffer.size());
        _readIndex += offset;
    }

    // assume the alignment of T is 1
    // assume T is not pointer
    template <typename T>
    void append(const T& t)
    {
        static_assert(std::is_pod_v<T>);
        static_assert(
            !std::is_pointer_v<T>);  // append pointer itself is not allowed
        append(&t, sizeof(T));
    }

    void append(const void* address, size_t size)
    {
        _buffer.insert(_buffer.end(),
                       static_cast<const char*>(address),
                       static_cast<const char*>(address) + size);
    }

    // assume the alignment of T is 1
    template <typename T>
    T take()
    {
        static_assert(std::is_pod_v<T>);
        assert(hasEnoughSizeToRead(sizeof(T)));
        char rowBuffer[sizeof(T)];
        memcpy(rowBuffer, rowReadData(), sizeof(T));
        seekReadOffset(sizeof(T));
        return reinterpret_cast<T&>(rowBuffer);
    }

    // assume the alignment of T is 1
    template <typename T>
    T peek() const
    {
        static_assert(std::is_pod_v<T>);
        assert(hasEnoughSizeToRead(sizeof(T)));
        char rowBuffer[sizeof(T)];
        memcpy(rowBuffer, rowReadData(), sizeof(T));
        return reinterpret_cast<T&>(rowBuffer);
    }

    const void* rowData() const
    {
        return static_cast<const void*>(_buffer.data());
    }

    void reserve(size_t size)
    {
        _buffer.reserve(size);
    }

    void shrinkToFit()
    {
        _buffer.shrink_to_fit();
    }

    // assume the alignment of T is 1
    // design for writing the buffer header after buffer header is wrote
    template <typename T>
    void rewriteFront(const T& t)
    {
        static_assert(std::is_pod_v<T>);
        assert(sizeof(t) <= _buffer.size());
        std::memcpy(rowData(), &t, sizeof(T));
    }

private:
    // return row data that corresponding to read index
    const void* rowReadData() const
    {
        return _buffer.data() + _readIndex;
    }

    // return if remain size if enough to read size-specificed data
    bool hasEnoughSizeToRead(size_t size) const
    {
        return _buffer.size() - _readIndex >= size;
    }

    void* rowData()
    {
        return static_cast<void*>(_buffer.data());
    }

private:
    std::vector<char> _buffer;
    size_t            _readIndex = 0;
};