#pragma once
#include "../../../src/log.h"
#include <map>
#include <string>

class Message
{
public:
    Message()          = default;
    Message(Message&&) = default;
    Message(std::initializer_list<std::pair<const std::string, std::string>>
                init_list);

    Message(std::map<std::string, std::string> map) : _map{std::move(map)} {}

    Message& operator=(Message&& m) = default;

    // insert or update if not exist
    void set(const std::string& key, const std::string& value)
    {
        _map.emplace(key, value);
    }

    // assert key exist and  get value of key
    std::string get(const std::string& key)
    {
        LOG_ASSERT(contains(key));
        return _map[key];
    }

    // check if key exist
    bool contains(const std::string& key) const
    {
        return _map.contains(key);
    }

    //  get value of key, insert empty string if key not exist
    std::string& operator[](const std::string& str)
    {
        return _map[str];
    }

    std::string toString() const;

    std::string toReadableString();

    // check if any message is contained
    static bool containsMessage(const std::string& string)
    {
        return getNextMessageLength(string) > 0;
    }

    // get one message from string, not modify string
    static Message fromString(const std::string& string);

    // get one message from string, extract string
    static Message extractString(std::string& string);

    const std::map<std::string, std::string>& rawData() const
    {
        return _map;
    }

    static Message createErrorMessage(std::string errorInfo)
    {
        Message m;
        m.set("status", "error");
        m.set("error", errorInfo);
        return std::move(m);
    }

    static Message createResultMessage(std::string result)
    {
        Message m;
        m.set("status", "ok");
        m.set("result", result);
        return std::move(m);
    }

private:
    static int getNextMessageLength(const std::string& string);

private:
    std::map<std::string, std::string> _map;
};