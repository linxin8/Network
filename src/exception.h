#pragma once

#include <exception>
#include <string>

class Exception : public std::exception
{
public:
    explicit Exception(std::string what);
    const char* what() const noexcept override
    {
        return _message.c_str();
    }
    const char* stackTrace() const noexcept
    {
        return _stackTrace.c_str();
    }

private:
    std::string _message;
    std::string _stackTrace;
};