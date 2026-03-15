#include "Time.h"

#include <iomanip>
#include <sstream>

namespace core::time {

    TimePoint Now()
    {
        return SteadyClock::now();
    }

    Milliseconds Since(TimePoint start)
    {
        return std::chrono::duration_cast<Milliseconds>(Now() - start);
    }

    MissionTime ToMissionTime(Milliseconds elapsed)
    {
        uint64_t totalSeconds = static_cast<uint64_t>(elapsed.count() / 1000);

        MissionTime out{};
        out.hours = static_cast<uint32_t>(totalSeconds / 3600);
        out.minutes = static_cast<uint32_t>((totalSeconds % 3600) / 60);
        out.seconds = static_cast<uint32_t>(totalSeconds % 60);

        return out;
    }

    std::string FormatMissionTime(const MissionTime& time)
    {
        std::ostringstream out;
        out << std::setw(2) << std::setfill('0') << time.hours << ":"
            << std::setw(2) << std::setfill('0') << time.minutes << ":"
            << std::setw(2) << std::setfill('0') << time.seconds;

        return out.str();
    }

    Milliseconds ToMilliseconds(const TimePoint& tp)
    {
        using namespace std::chrono;

        return duration_cast<milliseconds>(
            tp.time_since_epoch());
    }
}