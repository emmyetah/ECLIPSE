#pragma once

#include <cstdint>

namespace eclipse::telemetry {

    // Unique identifier for each telemetry metric in the system
 
    enum class MetricId : uint8_t {
        invalid = 0,

        // Environmental
        TempC_BME680,
        TempC_SCD30,
        TempC,
        HumidityRH_BME680,
        HumidityRH_SCD30,
        HumidityRH,
        PressureHpa,

        // Atmosphere / life support
        CO2ppm,

        // Radiation
        RadiationCpm,

        //Sensor Health Metrics
        BME680Health,
        SCD30Health,
        SystemHealth,

        // Sentinel value (not a real metric) 
        Count //number of enum entries
    };

}
