#pragma once
#include "type.h"
#include <functional>

class Channel : Noncopyable
{
public:
    // not take ownship of fd
    Channel(int fd);
    Channel(Channel&& right);
    ~Channel();

    int getListenEvents() const
    {
        return _listenEvent;
    }
    void setEventGenerated(int eventGenerated)
    {
        _eventGenerated = eventGenerated;
    }
    int getFd() const
    {
        return _fd;
    }

    void update();

    void setOnRead(std::function<void()> onRead)
    {
        _onRead = std::move(onRead);
    }
    void setOnWrite(std::function<void()> onWrite)
    {
        _onWrite = std::move(onWrite);
    }
    void setOnClose(std::function<void()> onClose)
    {
        _onClose = std::move(onClose);
    }
    void setOnError(std::function<void(int errorNo)> onError)
    {
        _onError = std::move(onError);
    }

    constexpr bool isInEpoll() const
    {
        return _index != -1;
    }

    void enableRead();
    void disableRead();
    void enableWrite();
    void enableReadAndWrite();
    void disableWrite();
    void disableReadAndWrite();
    bool isEnableRead() const;
    bool isEnbleWrite() const;
    // get index of pool channel list.
    // if not set, return -1
    int getIndex() const
    {
        return _index;
    }

    // set index of pool channel list
    void setIndex(int index)
    {
        _index = index;
    }

    void handleEvent();

private:
    int                              _listenEvent;     // listened by epoll
    int                              _eventGenerated;  // set by epoll
    int                              _fd;
    std::function<void()>            _onRead;
    std::function<void()>            _onWrite;
    std::function<void()>            _onClose;
    std::function<void(int errorNo)> _onError;
    int                              _index;  //  index of poll channel list
};