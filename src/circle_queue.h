#pragma once
#include <cassert>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>

template <typename T, size_t size, bool>
class _CircleQueue;

// not thread safe, only allow static pod element
template <typename T, size_t size>
class _CircleQueue<T, size, true>
{
    static_assert(size > 1);
    static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
    static_assert(!std::is_pointer_v<T>);

public:
    ~_CircleQueue()
    {
        delete[] _data;
    }
    constexpr size_t getCapacity() const
    {
        return size;
    }

    // test if is empty
    bool isEmpty() const
    {
        return _length == 0;
    }
    // test if is full
    bool isFull() const
    {
        return _length == size;
    }

    T& getFront()
    {
        return _data[_start];
    }

    const T& getFront() const
    {
        return _data[_start];
    }

    // remove and return the first item
    T take()
    {
        assert(!isEmpty());
        T& ret = _data[_start];
        _start = next(_start);
        _length--;
        return std::move(ret);
    }
    // add element
    void push(const T& x)
    {
        assert(!isFull());
        _data[_end] = x;
        _end        = next(_end);
        _length++;
    }

    // add element
    void push(T&& x)
    {
        assert(!isFull());
        _data[_end] = std::move(x);
        _end        = next(_end);
        _length++;
    }

    void pop()
    {
        assert(!isEmpty());
        _start = next(_start);
        _length--;
    }

    size_t getSize() const
    {
        return _length;
    }

    void swap(_CircleQueue<T, size, true>& right)
    {
        std::swap(_data, right._data);
        std::swap(_start, right._start);
        std::swap(_end, right._end);
        std::swap(_length, right._length);
    }

private:
    int next(int x) const
    {
        int ret = x + 1;
        if (ret >= size)
        {
            ret -= size;
        }
        return ret;
    }

private:
    T*  _data   = new T[size];
    int _start  = 0;
    int _end    = 0;
    int _length = 0;
};

// not thread safe, only allow static non-pod element
template <typename T, size_t size>
class _CircleQueue<T, size, false>
{
    static_assert(size > 1);
    static_assert(
        !(std::is_standard_layout_v<T> &&
          std::is_trivial_v<T>));  // for performace, use CircleQueue instead

public:
    ~_CircleQueue()
    {
        delete[] _data;
    }
    constexpr size_t getCapacity() const
    {
        return size;
    }

    // test if is empty
    bool isEmpty() const
    {
        return _length == 0;
    }
    // test if is full
    bool isFull() const
    {
        return _length == size;
    }
    // remove and return the first item
    T take()
    {
        assert(!isEmpty());
        T& x   = get(_start);
        _start = next(_start);
        _length--;
        T result = std::move(x);
        x.~T();
        return std::move(result);
    }

    void pop()
    {
        assert(!isEmpty());
        get(_start).~T();
        _start = next(_start);
        _length--;
    }
    // add element
    void push(const T& x)
    {
        assert(!isFull());
        construct(_end, x);
        _end = next(_end);
        _length++;
    }

    // add element
    void push(T&& x)
    {
        assert(!isFull());
        construct(_end, std::move(x));
        _end = next(_end);
        _length++;
    }

    size_t getSize() const
    {
        return _length;
    }

    T& getFront()
    {
        return get(_start);
    }

    const T& getFront() const
    {
        return get(_start);
    }

    void swap(_CircleQueue<T, size, false>& right)
    {
        std::swap(_data, right._data);
        std::swap(_start, right._start);
        std::swap(_end, right._end);
        std::swap(_length, right._length);
    }

    void clear()
    {
        while (!isEmpty())
        {
            pop();
        }
    }

private:
    int next(int x) const
    {
        int ret = x + 1;
        if (ret >= size)
        {
            ret -= size;
        }
        return ret;
    }

    void construct(int x, const T& value)
    {
        new (&get(x)) T(value);
    }

    void construct(int x, T&& value)
    {
        new (&get(x)) T(std::move(value));
    }

    void destruct(int x)
    {
        get(x).~T();
    }

    T& get(int x)
    {
        assert(x >= 0 && x < size);
        return *reinterpret_cast<T*>(_data + x * sizeof(T));
    }

private:
    char* _data   = new char[sizeof(T) * size];
    int   _start  = 0;
    int   _end    = 0;
    int   _length = 0;
};

template <typename T, size_t size>
using CircleQueue =
    _CircleQueue<T, size, std::is_standard_layout_v<T> && std::is_trivial_v<T>>;

// not thread safe
class CircleByteStreamQueue
{
public:
    CircleByteStreamQueue(size_t size) :
        _size{size}, _data{new char[_size]}, _start{0}, _end{0}, _length{0}
    {
    }

    ~CircleByteStreamQueue()
    {
        if (_data != nullptr)
        {
            delete[] _data;
        }
    }

    CircleByteStreamQueue(CircleByteStreamQueue&& right) :
        _size{right._size},
        _start{right._start},
        _end{right._end},
        _length{right._length}
    {
        _data       = right._data;
        right._data = nullptr;
    }

    size_t getCapacity() const
    {
        return _size;
    }

    // test if is empty
    bool isEmpty() const
    {
        return _length == 0;
    }
    // test if is full
    bool isFull() const
    {
        return _length == _size;
    }

    // get the address of first element
    const char* getFirstAddress() const
    {
        return &_data[_start];
    }

    // get the address of first element
    char* getFirstAddress()
    {
        return &_data[_start];
    }

    // get the address of circle buffer
    const char* getRowData() const
    {
        return &_data[0];
    }

    // return the first element while not remove the element
    char peek() const
    {
        assert(!isEmpty());
        return _data[_start];
    }

    // remove and return the first item
    char take()
    {
        assert(!isEmpty());
        char ret = _data[_start];
        _start   = next(_start, 1);
        _length--;
        return ret;
    }

    // add element
    void push(char ch)
    {
        assert(!isFull());
        _data[_end] = ch;
        _end        = next(_end, 1);
        _length++;
    }

    // return the size that can be stored
    size_t getAvailableSize()
    {
        return _size - _length;
    }

    // assume size is enougth
    template <typename T>
    void push(const T& x)
    {
        static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
        static_assert(!std::is_pointer_v<T>);
        assert(getAvailableSize() >= sizeof(T));
        push(&x, sizeof(T));
    }

    // push data stream with maximun size
    // reutrn actually size pushed
    size_t push(const void* data, size_t maxSize)
    {
        size_t actualSize    = min(maxSize, getAvailableSize());
        size_t toEndSize     = _size - _end;
        size_t firstCopySize = min(actualSize, toEndSize);
        std::memcpy(&_data[_end], data, firstCopySize);
        if (actualSize > firstCopySize)
        {
            std::memcpy(&_data[0],
                        static_cast<const char*>(data) + firstCopySize,
                        actualSize - firstCopySize);
        }
        _end = next(_end, actualSize);
        _length += actualSize;
        return actualSize;
    }

    // return continous size of data can be read from first element
    size_t getContinousSize() const
    {
        if (_start > _end)
        {
            return _size - _start;
        }
        return _end - _start;
    }

    // remove continous size of data can be read from first element
    void popContinouData()
    {
        if (_start <= _end)
        {
            _start  = 0;
            _end    = 0;
            _length = 0;
        }
        else
        {
            _start  = 0;
            _length = _end - _start;
        }
    }

    // pop fixed size data
    void popN(int popSize)
    {
        assert(!isEmpty());
        assert(_length >= popSize);
        _start = next(_start, popSize);
        _length -= popSize;
    }

    // reutrn the size of element stored
    size_t getSize() const
    {
        return _length;
    }

    // new size must not less than current size
    // all old data are retained
    void resetCapcity(size_t newSize)
    {
        assert(_length <= newSize);
        auto newData = new char[newSize];
        auto csize   = getContinousSize();
        memcpy(newData, getFirstAddress(), getContinousSize());
        if (_end < _start)
        {
            memcpy(newData + csize, getRowData(), getSize() - csize);
        }
        delete[] _data;
        _data  = newData;
        _start = 0;
        _end   = next(_start, _length);
        _size  = newSize;
    }

    // {wirting address, max size}
    // used for direct wirte for avoid copying buffer data
    // must call endDirectWrite once wirting is finished
    std::pair<char*, size_t> beginDirectWrite()
    {
        if (_start > _end)
        {
            return {_data + _end, _start - _end};
        }
        return {_data + _end, _size - _end};
    }

    // used for direct wirte for avoid copying buffer data
    // must be called after beginDirectWrite
    // writtenSize is size of data that have been writed
    void endDirectWrite(size_t writtenSize)
    {
        _end = next(_end, writtenSize);
        _length += writtenSize;
        assert(_length <= _size);
    }

private:
    size_t min(size_t a, size_t b)
    {
        return a < b ? a : b;
    }

    size_t next(size_t x, size_t n) const
    {
        size_t ret = x + n;
        if (ret >= _size)
        {
            ret -= _size;
        }
        return ret;
    }

private:
    size_t _size;
    char*  _data;
    size_t _start;
    size_t _end;
    size_t _length;
};