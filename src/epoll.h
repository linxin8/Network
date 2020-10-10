#pragma once

#include <vector>

class Channel;

class EPoll
{
public:
    EPoll();
    ~EPoll();

    void pool(int msecond, std::vector<Channel*> channel);

private:
private:
    int                             _fd;
    std::vector<struct epoll_event> _eventBuffer;
};