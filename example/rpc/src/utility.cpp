#include "utility.h"
#include "../../../src/log.h"
#include "../../../src/time.h"

int64_t generateID(std::size_t machineId, std::size_t sequence)
{
    LOG_ASSERT(machineId < 16);   // 4 bits
    LOG_ASSERT(sequence < 1024);  // 10 bits
    static const int64_t timestamp = TimePoint::now().microTimeSinceEpoch();
    return (timestamp << 14) + (machineId << 10) + sequence;
}