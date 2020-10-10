#pragma once

#include "type.h"
#include <array>
#include <cassert>
#include <netinet/in.h>
#include <string_view>
#include <utility>

// struct sockaddr_in {
//     short            sin_family;   // e.g. AF_INET
//     unsigned short   sin_port;     // e.g. htons(3490)
//     struct in_addr   sin_addr;     // see struct in_addr, below
//     char             sin_zero[8];  // zero this if you want to
// };

// struct in_addr {
//     unsigned long s_addr;  // load with inet_aton()
// };

class InetAddress
{
private:
    /* data */
public:
    InetAddress(uint16_t port, bool isLoopBackOnly = false, bool isIpv6 = false);

    // resovle hostname, return (is successful, result)
    static std::pair<bool, InetAddress>
    resolve(std::string_view hostname, uint16_t port, bool isLoopBackOnly = false, bool isIpv6 = false);

    constexpr bool isIp4() const
    {
        return _addr.sin_family == AF_INET;
    }
    constexpr bool isIp6() const
    {
        return _addr.sin_family == AF_INET6;
    };

    uint16_t getPort() const
    {
        return ntohs(_addr.sin_port);
    }

    constexpr uint32_t getIp4() const
    {
        return _addr.sin_addr.s_addr;
    }

    constexpr socklen_t getSocketAddressSize() const
    {
        return isIp4() ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    }

    constexpr const sockaddr& getSocketAddress() const
    {
        return reinterpret_cast<const sockaddr&>(_addr);
    }

    constexpr sockaddr& getSocketAddress()
    {
        return reinterpret_cast<sockaddr&>(_addr);
    }

    // universal api for both ip4 and ip6
    std::string getIpString() const;
    // only for ip4
    std::string getIp4String() const;
    // only for ip6
    std::string getIp6String() const;

    constexpr static InetAddress getInvalidAddress()
    {
        return {};
    }

private:
    constexpr InetAddress() : _addr6{} {};
    constexpr InetAddress(sockaddr_in address) : _addr{address} {};
    constexpr InetAddress(sockaddr_in6 address) : _addr6{address} {};
    constexpr InetAddress(sockaddr address)
        : _addr{
              reinterpret_cast<sockaddr_in&>(address)  // it is safe to cast
          } {};
    union
    {
        struct sockaddr_in  _addr;   // ip v4
        struct sockaddr_in6 _addr6;  // ip v6
    };
};