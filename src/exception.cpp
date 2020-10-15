#include "exception.h"
#include "thread.h"

Exception::Exception(std::string what) :
    _message(std::move(what)), _stackTrace(CurrentThread::getStackTrace(true))
{
}
