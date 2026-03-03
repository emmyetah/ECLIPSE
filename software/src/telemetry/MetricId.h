#pragma once

#include <cstdint>

namespace eclipse::telemetry {

    // Unique identifier for each telemetry metric in the system
 
    enum class MetricId : uint8_t {
        invalid = 0,

        // Environmental
        TempC = 1,
        HumidityRH = 2,
        PressureHpa = 3,

        // Atmosphere / life support
        CO2ppm = 4,

        // Radiation
        RadiationCpm = 5,
        // Sentinel value (not a real metric) 
        Count = 6 //number of enum entries
    };

}
