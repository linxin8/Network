#include "message.h"

Message::Message(
    std::initializer_list<std::pair<const std::string, std::string>>
        init_list) :
    _map{
        std::move(init_list),
    }
{
}

std::string Message::toString() const
{
    std::string header;
    std::string body;
    for (auto& [k, v] : _map)
    {
        body.append(std::to_string(k.size()));
        body.push_back(',');
        body.append(k);
        body.push_back(',');
        body.append(std::to_string(v.size()));
        body.push_back(',');
        body.append(v);
        body.push_back(',');
    }
    header.append(std::to_string(body.size()));
    header.push_back(',');
    return header + body;
}

int Message::getNextMessageLength(const std::string& string)
{
    if (string.empty())
    {
        return -1;
    }
    int n = 0;
    for (auto& c : string)
    {
        if (c == ',')
        {
            break;
        }
        if (c < '0' || c > '9')
        {
            LOG_ERROR() << "error message format" << string;
            return -2;
        }
        n = n * 10 + c - '0';
    }
    LOG_ASSERT(n > 0);
    return n;
}

Message Message::fromString(const std::string& string)
{
    int n = getNextMessageLength(string);
    LOG_ASSERT(n > 0);
    int                                p = 0;
    std::map<std::string, std::string> map;
    while (string[p] != ',')
    {
        p++;
    }
    p++;
    std::string key;
    std::string value;
    while (p < n)
    {
        int ks = 0;
        int vs = 0;
        while (string[p] != ',')
        {
            LOG_ASSERT(string[p] >= '0' && string[p] <= '9');
            ks = ks * 10 + string[p] - '0';
            p++;
        }
        p++;
        key = string.substr(p, ks);
        p += ks + 1;
        while (string[p] != ',')
        {
            LOG_ASSERT(string[p] >= '0' && string[p] <= '9');
            vs = vs * 10 + string[p] - '0';
            p++;
        }
        p++;
        value = string.substr(p, vs);
        p += vs + 1;
        map.emplace(std::move(key), std::move(value));
    }
    return {map};
}

Message Message::extractString(std::string& string)
{
    int n = getNextMessageLength(string);
    LOG_ASSERT(n > 0);
    int                                p = 0;
    std::map<std::string, std::string> map;
    while (string[p] != ',')
    {
        p++;
    }
    p++;
    std::string key;
    std::string value;
    while (p < n)
    {
        int ks = 0;
        int vs = 0;
        while (string[p] != ',')
        {
            LOG_ASSERT(string[p] >= '0' && string[p] <= '9');
            ks = ks * 10 + string[p] - '0';
            p++;
        }
        p++;
        key = string.substr(p, ks);
        p += ks + 1;
        while (string[p] != ',')
        {
            LOG_ASSERT(string[p] >= '0' && string[p] <= '9');
            vs = vs * 10 + string[p] - '0';
            p++;
        }
        p++;
        value = string.substr(p, vs);
        p += vs + 1;
        map.emplace(std::move(key), std::move(value));
    }
    string = string.substr(p);
    return {map};
}

std::string Message::toReadableString()
{
    std::string result;
    result.append("[");
    for (auto& [k, v] : _map)
    {
        result.append(k);
        result.append(":");
        result.append(v);
        result.push_back(',');
    }
    if (result.back() == ',')
    {
        result.pop_back();
    }
    result.append("]");
    return std::move(result);
}