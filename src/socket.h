#pragma once

class socket
{
private:
    /* data */
public:
    explicit socket(int fd) : _fd{fd} {}
    ~socket();

private:
    const int _fd;
};
