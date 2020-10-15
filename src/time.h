#pragma once
#include <cstdint>
#include <time.h>

class TimePoint
{
public:
    constexpr TimePoint() : _epochTime{} {};
    constexpr TimePoint(int64_t timeEpoch) : _epochTime{timeEpoch} {};
    static TimePoint now();
    bool             isValid() const
    {
        return _epochTime > 0;
    }

    // compiler not support yet
    // bool operator<=>(TimePoint time)
    // {
    //     return _timeEpoch <=> time._timeEpoch;
    // }
    bool operator<(TimePoint time) const
    {
        return _epochTime < time._epochTime;
    }
    bool operator<=(TimePoint time) const
    {
        return _epochTime <= time._epochTime;
    }
    bool operator==(TimePoint time) const
    {
        return _epochTime == time._epochTime;
    }
    bool operator!=(TimePoint time) const
    {
        return _epochTime != time._epochTime;
    }
    bool operator>(TimePoint time) const
    {
        return _epochTime > time._epochTime;
    }
    bool operator>=(TimePoint time) const
    {
        return _epochTime >= time._epochTime;
    }

    int64_t microTimeSinceEpoch() const
    {
        return _epochTime;
    }

    TimePoint add(double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * _microSecondPerSecond);
        return {_epochTime + delta};
    }
    TimePoint sub(double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * _microSecondPerSecond);
        return {_epochTime - delta};
    }
    double difference(TimePoint time) const
    {
        return static_cast<double>(_epochTime - time._epochTime);
    }
    int hours() const
    {
        constexpr auto microSecondPerHour = _microSecondPerSecond * 60 * 60;
        constexpr auto microSecondPerDay  = microSecondPerHour * 24;
        auto           chinaTime =
            8 + _epochTime % microSecondPerDay / microSecondPerHour;
        if (chinaTime >= 24)
        {
            chinaTime -= 24;
        }
        return chinaTime;  // convert to china time
    }
    int minute() const
    {
        constexpr auto microSecondPerMinute = _microSecondPerSecond * 60;
        constexpr auto microSecondPerHour   = microSecondPerMinute * 60;
        return _epochTime % microSecondPerHour / microSecondPerMinute;
    }
    int second() const
    {
        constexpr auto microSecondPerMinute = _microSecondPerSecond * 60;
        return _epochTime % microSecondPerMinute / _microSecondPerSecond;
    }

private:
    static constexpr int64_t _microSecondPerSecond = 1000 * 1000;
    //  time since epoch
    int64_t _epochTime;
};