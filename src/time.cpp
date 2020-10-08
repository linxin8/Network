#include "time.h"
#include <stdio.h>
#include <sys/time.h>

TimePoint TimePoint::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return TimePoint(seconds * _microSecondPerSecond + tv.tv_usec);
}