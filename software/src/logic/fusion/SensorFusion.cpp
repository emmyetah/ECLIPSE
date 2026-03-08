#include "SensorFusion.h"

#include <cmath>

namespace eclipse::logic::fusion {

    //gets both temp values from the most recent snapshot and returns these values + the max difference.
    SensorFusion::FusionResult SensorFusion::FuseTemperature(
        const telemetry::TelemetrySnapshot& snapshot,
        double maxDifference // max difference was hardcoded as 1 in sensorFusion.h. could add this to the configuration.
    ) {
        const std::optional<double> bme = snapshot.Value(telemetry::MetricId::TempC_BME680);
        const std::optional<double> scd = snapshot.Value(telemetry::MetricId::TempC_SCD30);

        return FusePair(bme, scd, maxDifference);
    }

    //gets both humidity values from the most recent snapshot & returns these values + the max difference
    SensorFusion::FusionResult SensorFusion::FuseHumidity(
        const telemetry::TelemetrySnapshot& snapshot,
        double maxDifference
    ) {
        const std::optional<double> bme = snapshot.Value(telemetry::MetricId::HumidityRH_BME680);
        const std::optional<double> scd = snapshot.Value(telemetry::MetricId::HumidityRH_SCD30);

        return FusePair(bme, scd, maxDifference);
    }

    SensorFusion::FusionResult SensorFusion::FusePair(
        std::optional<double> a,
        std::optional<double> b,
        double maxDifference
    ) {
        FusionResult result{};

        //if both values are available, compare and fuse them
        if (a.has_value() && b.has_value()) {
            //std::fabs removes negative sign, * is for dereferencing the double to access the actual value due to optional<>
            const double diff = std::fabs(*a - *b);

            //if the readings differ too much, mark mismatch.
            if (diff > maxDifference) {
                result.mismatch = true;

                //Use SCD30 sensor value conventionally
                result.value = *b;
                return result;
            }

            //if they agree closely enough, average them.
            result.value = (*a + *b) / 2.0;
            return result;
        }

        //if only one value is available, fall back to it.
        if (a.has_value()) {
            result.value = *a;
            result.usedFallback = true;
            return result;
        }

        if (b.has_value()) {
            result.value = *b;
            result.usedFallback = true;
            return result;
        }

        //neither value is available.
        return result;
    }

}