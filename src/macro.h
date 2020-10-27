#pragma once
#include "log.h"
#include "thread.h"
#include <cassert>

#define LOG_ASSERT(x)                                                          \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            LOG_ERROR() << "error on assertion" << #x << x                     \
                        << CurrentThread::getStackTrace();                     \
            assert(false);                                                     \
        }                                                                      \
    }