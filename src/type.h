#pragma once

class Noncopyable
{
public:
    Noncopyable(const Noncopyable&) = delete;
    void operator=(const Noncopyable&) = delete;

protected:
    constexpr Noncopyable() = default;
    ~Noncopyable()          = default;
};