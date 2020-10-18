#pragma once
#include <functional>

class Channel
{
public:
    // not take ownship of fd
    Channel(int fd);
    ~Channel();

    int getEvents() const
    {
        return _events;
    }
    void setEvents(int events)
    {
        _events = events;
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
        _onWrite = std::move(_onWrite);
    }
    void setOnClose(std::function<void()> onClose)
    {
        _onClose = std::move(_onClose);
    }
    void setOnError(std::function<void()> onError)
    {
        _onError = std::move(_onError);
    }

    constexpr bool isInEpoll() const
    {
        return _index != -1;
    }

    void enableRead();
    void disableRead();
    void enableWrite();
    void disableWrite();
    bool isEnableRead();
    bool isEnbleWrite();
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
    int                   _events;
    int                   _fd;
    std::function<void()> _onRead;
    std::function<void()> _onWrite;
    std::function<void()> _onClose;
    std::function<void()> _onError;
    int                   _index;  //  index of poll channel list
};