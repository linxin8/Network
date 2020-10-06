#include "file.h"
#include <cassert>

WriteOnlyFile::WriteOnlyFile(const char* path) : _file{fopen(path, "w")}, _buffer{}, _byteWrited{0}
{
    assert(_file);
    setbuffer(_file, _buffer, sizeof(_buffer));
}

WriteOnlyFile::~WriteOnlyFile()
{
    fclose(_file);  // fclose will automatically flush
}

void WriteOnlyFile::append(const char* data, size_t size)
{
    size_t n = fwrite_unlocked(data, 1, size, _file);
    while (n < size)
    {
        size_t next = fwrite_unlocked(data + n, 1, size - n, _file);
        if (next == 0)
        {
            int err = ferror(_file);
            if (err)
            {
                fprintf(stderr, "WriteOnlyFile::append failed, error no: %d\n", err);
                break;
            }
        }
        n += next;
    }
    _byteWrited += n;
}

void WriteOnlyFile::flush()
{
    fflush(_file);
}