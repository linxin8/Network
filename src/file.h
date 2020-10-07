#pragma once
#include "type.h"
#include <cstddef>
#include <cstdio>

// not thread safe
class WriteOnlyFile : Noncopyable
{
public:
    WriteOnlyFile(const char* path);
    ~WriteOnlyFile();
    void append(const char* data, size_t size);
    void flush();
    int  byteWrited() const
    {
        return _byteWrited;
    }

private:
    FILE* _file;
    // char* _buffer = new char[1024 * 1024]; not need alloc buffer manully
    int _byteWrited;
};