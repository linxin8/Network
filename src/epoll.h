#pragma once

#include <vector>

class Channel;

class EPoll
{
public:
    EPoll();
    ~EPoll();

    void pool(int msecond, std::vector<Channel*>& channel);
    void add(Channel* channel);
    void modify(Channel* channel);
    void remove(Channel* channel);

private:
    void control(int operation, Channel* channel);

private:
    int                             _fd;
    std::vector<struct epoll_event> _eventBuffer;
    std::vector<Channel*>           _channelVector;
};