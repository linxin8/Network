#include "file.h"
#include <cassert>
#include <cstdlib>
#include <sys/stat.h>

WriteOnlyFile::WriteOnlyFile(const char* path) : _file{fopen(path, "w")}, _byteWrited{0}
{
    assert(_file);
    struct stat stats;
    if (fstat(fileno(_file), &stats) == -1)
    {  // POSIX only
        std::perror("fstat");
        std::abort();
    }
    if (std::setvbuf(_file, NULL, _IOFBF, stats.st_blksize) != 0)
    {
        std::perror("setvbuf failed");  // POSIX version sets errno
        std::abort();
    }
    // not need alloc buffer manully
    // setbuffer(_file, _buffer, sizeof(_buffer));
}

WriteOnlyFile::~WriteOnlyFile()
{
    fclose(_file);  // fclose will automatically flush
    // delete _buffer;
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