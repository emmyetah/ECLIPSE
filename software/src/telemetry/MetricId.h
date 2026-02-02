#pragma once

#include <cstdint>

namespace telemetry {

    // Unique identifier for each telemetry metric in the system
 
    enum class MetricId : uint8_t {
        invalid = 0,

        // Environmental
        temperature,
        pressure,
        humidity,

        // Atmosphere / life support
        co2,
        oxygen,

        // Radiation
        radiation,

        // System / power (optional / future)
        battery_level,
        power_consumption,

        // Sentinel value (not a real metric)
        count
    };

}
