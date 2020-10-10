#include "inet_address.h"
#include "log.h"
#include <arpa/inet.h>
#include <cassert>
#include <charconv>
#include <endian.h>
#include <netdb.h>
#include <string.h>

static_assert(sizeof(InetAddress) == sizeof(sockaddr_in6));

InetAddress::InetAddress(uint16_t port, bool isLoopBackOnly, bool isIpv6)
{
    static_assert(__builtin_offsetof(InetAddress, _addr) == 0);
    static_assert(__builtin_offsetof(InetAddress, _addr6) == 0);
    static_assert(__builtin_offsetof(sockaddr_in, sin_family) == __builtin_offsetof(sockaddr_in6, sin6_family));
    static_assert(__builtin_offsetof(sockaddr_in, sin_port) == __builtin_offsetof(sockaddr_in6, sin6_port));
    if (isIpv6)
    {
        bzero(&_addr6, sizeof(_addr6));
        _addr6.sin6_family = AF_INET6;
        in6_addr ip        = isLoopBackOnly ? in6addr_loopback : in6addr_any;
        _addr6.sin6_addr   = ip;
        _addr6.sin6_port   = htons(port);
    }
    else
    {
        bzero(&_addr, sizeof(_addr));
        _addr.sin_family      = AF_INET;
        in_addr_t ip          = isLoopBackOnly ? INADDR_LOOPBACK : INADDR_ANY;
        _addr.sin_addr.s_addr = htonl(ip);
        _addr.sin_port        = htons(port);
    }
}

std::string InetAddress::getIpString() const
{
    return isIp4() ? getIp4String() : getIp6String();
}

std::string InetAddress::getIp4String() const
{
    assert(isIp4());
    char buffer[32];
    if (inet_ntop(AF_INET, &_addr.sin_addr, buffer, sizeof(buffer)) == nullptr)
    {
        LOG_ERROR() << std::strerror(errno);
    }
    return buffer;
}

std::string InetAddress::getIp6String() const
{
    assert(isIp6());
    char buffer[64];
    if (inet_ntop(AF_INET6, &_addr6.sin6_addr, buffer, sizeof(buffer)) == nullptr)
    {
        LOG_ERROR() << std::strerror(errno);
    }
    return buffer;
}

std::pair<bool, InetAddress>
InetAddress::resolve(std::string_view hostname, uint16_t port, bool isLoopBackOnly, bool isIpv6)
{
    /* Structure to contain information about address of a service provider.  */
    // struct addrinfo
    // {
    //     int              ai_flags;     /* Input flags.  */
    //     int              ai_family;    /* Protocol family for socket.  */
    //     int              ai_socktype;  /* Socket type.  */
    //     int              ai_protocol;  /* Protocol for socket.  */
    //     socklen_t        ai_addrlen;   /* Length of socket address.  */
    //     struct sockaddr* ai_addr;      /* Socket address for socket.  */
    //     char*            ai_canonname; /* Canonical name for service location.  */
    //     struct addrinfo* ai_next;      /* Pointer to next in list.  */
    // };
    static thread_local char buffer[1024]{};

    addrinfo hint;
    bzero(&hint, sizeof(hint));
    hint.ai_family = AF_UNSPEC;  // AF_UNSPEC for either ip4 or ip6
                                 // AF_INET for ip4
                                 // AF_INET6 for ip6

    hint.ai_socktype = 0;  // 0 for either tcp or udp
                           // SOCK_STREAM for tcp
                           // SOCK_DGRAM for udp

    hint.ai_flags = 0;  // other option

    hint.ai_protocol = 0;  // 0 for any protocal

    addrinfo* result;
    char      port_str[10];
    auto      r = std::to_chars(port_str, port_str + 10, port);
    assert(r.ec == std::errc{});                                        // assume no error
    assert(r.ptr < port_str + sizeof(port_str) / sizeof(port_str[0]));  // assume not overflow
    *r.ptr = '\0';
    if (getaddrinfo(hostname.begin(), port_str, &hint, &result) != 0)
    {  // error
        LOG_DEBUG() << "cannto resove address:" << hostname.begin() << port_str << std::strerror(errno);
        return {false, {}};
    }
    assert(result != nullptr);
    InetAddress address{*result->ai_addr};
    freeaddrinfo(result);
    return {true, {address}};
}