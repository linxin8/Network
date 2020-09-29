#include "log.h"
#include <iostream>

Logger::Logger()
{
    _stream.rdbuf(std::cout.rdbuf());
    LineLogger(__FILE__, __PRETTY_FUNCTION__, __LINE__);
}
