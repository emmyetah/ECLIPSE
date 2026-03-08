#pragma once

#include <optional>

#include "../../telemetry/TelemetrySnapshot.h"

namespace eclipse::logic::fusion {

    //represents the result of fusing two redundant sensor readings
    struct FusionResult {
        std::optional<double> value; //final fused value, if one could be produced
        bool usedFallback = false;   //true if only one valid sensor was used
        bool mismatch = false;       //true if both sensors were valid but disagreed too much
    };

    //provides helper functions for combining sensor readings
    class SensorFusion {
    public:
        //fuses the latest temperature values from BME680 and SCD30
        static FusionResult FuseTemperature(
            const telemetry::TelemetrySnapshot& snapshot,
            double maxDifference = 1.0 // hard coded maximum difference is 1 degree. they shouldn't differe more than this
        );

        //fuses the latest humidity values from BME680 and SCD30
        static FusionResult FuseHumidity(
            const telemetry::TelemetrySnapshot& snapshot,
            double maxDifference = 5.0
        );

    private:
        //shared helper for fusing two optional values with a mismatch tolerance
        static FusionResult FusePair(
            std::optional<double> a,
            std::optional<double> b,
            double maxDifference
        );
    };

}