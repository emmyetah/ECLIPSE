#pragma once

#include "../../telemetry/TelemetrySnapshot.h"
#include "../fusion/SensorFusion.h"
#include "SensorStatus.h"
#include "SystemStatus.h"

namespace eclipse::logic::health {

    //monitors the health of individual sensors and the overall system
    class SensorHealthMonitor {
    public:
        //evaluates the current health of the BME680 sensor. checks every metric, if any channel is stale or invalid then the status will be affected.
        static SensorStatus EvaluateBME680(
            const telemetry::TelemetrySnapshot& snapshot,
            const core::time::TimePoint& now,
            core::time::Duration staleAfter
        );

        //evaluates the current health of the SCD30 sensor
        static SensorStatus EvaluateSCD30(
            const telemetry::TelemetrySnapshot& snapshot,
            const core::time::TimePoint& now,
            core::time::Duration staleAfter
        );

        //derives the overall system status from the two sensor statuses
        static SystemStatus EvaluateSystemStatus(
            SensorStatus bme680Status,
            SensorStatus scd30Status
        );

    private:
        //shared helper for checking one raw telemetry channel
        static SensorStatus EvaluateChannel(
            const telemetry::TelemetrySnapshot& snapshot,
            telemetry::MetricId metric,
            const core::time::TimePoint& now,
            core::time::Duration staleAfter
        );
    };

}