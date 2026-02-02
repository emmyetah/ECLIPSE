#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace core::time {

    // Steady clock for durations & telemetry timing (clock that never goes backwards)
    using SteadyClock = std::chrono::steady_clock;

    // System clock for logs
    using SystemClock = std::chrono::system_clock;

    using TimePoint = SteadyClock::time_point;

    // Time units
    using Milliseconds = std::chrono::milliseconds;
    using Seconds = std::chrono::seconds;

    // -------- Basic helpers --------

    // Returns current steady-clock time
    TimePoint Now();

    //Duration since a given time point
    Milliseconds Since(TimePoint start);

    // -------- Mission time --------
    // 
    // Represents elapsed mission time since start
    struct MissionTime {
        uint32_t hours;
        uint32_t minutes;
        uint32_t seconds;
    };

    // helper that converts elapsed milliseconds into mission time (HH:MM:SS)
    MissionTime ToMissionTime(Milliseconds elapsed);

    // formats mission time as string 
    std::string FormatMissionTime(const MissionTime& time);

} 
