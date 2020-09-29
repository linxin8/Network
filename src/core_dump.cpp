#include "core_dump.h"
#include <sys/resource.h>

void enable_core_dump()
{
    // core dumps may be disallowed by parent of this process; change that
    struct rlimit core_limits;
    core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &core_limits);
}
struct _enable_core_dump
{
    _enable_core_dump()
    {
        enable_core_dump();
    }
} _enable_core_dump_;