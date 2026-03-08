#include "SensorHealthMonitor.h"

namespace eclipse::logic::health {


    SensorStatus SensorHealthMonitor::EvaluateBME680(
        const telemetry::TelemetrySnapshot& snapshot,
        const core::time::TimePoint& now,
        core::time::Duration staleAfter //how long ago did the senssor update?
    ) {
        const SensorStatus tempStatus = EvaluateChannel(snapshot, telemetry::MetricId::TempC_BME680, now, staleAfter);
        const SensorStatus humidityStatus = EvaluateChannel(snapshot, telemetry::MetricId::HumidityRH_BME680, now, staleAfter);
        const SensorStatus pressureStatus = EvaluateChannel(snapshot, telemetry::MetricId::PressureHpa, now, staleAfter);

        //if any core channel is offline, the whole sensor is offline
        if (tempStatus == SensorStatus::Offline ||
            humidityStatus == SensorStatus::Offline ||
            pressureStatus == SensorStatus::Offline) {
            return SensorStatus::Offline;
        }

        //if any core channel is invalid, mark the sensor invalid
        if (tempStatus == SensorStatus::Invalid ||
            humidityStatus == SensorStatus::Invalid ||
            pressureStatus == SensorStatus::Invalid) {
            return SensorStatus::Invalid;
        }

        //if any core channel is stale, mark the sensor stale
        if (tempStatus == SensorStatus::Stale ||
            humidityStatus == SensorStatus::Stale ||
            pressureStatus == SensorStatus::Stale) {
            return SensorStatus::Stale;
        }

        //check for disagreement with the SCD30 on duplicated channels
        const auto tempFusion = fusion::SensorFusion::FuseTemperature(snapshot);
        const auto humidityFusion = fusion::SensorFusion::FuseHumidity(snapshot);

        if (tempFusion.mismatch || humidityFusion.mismatch) {
            return SensorStatus::Mismatch;
        }

        
        return SensorStatus::Ok;
    }

    SensorStatus SensorHealthMonitor::EvaluateSCD30(
        const telemetry::TelemetrySnapshot& snapshot,
        const core::time::TimePoint& now,
        core::time::Duration staleAfter
    ) {
        const SensorStatus tempStatus =
            EvaluateChannel(snapshot, telemetry::MetricId::TempC_SCD30, now, staleAfter);

        const SensorStatus humidityStatus =
            EvaluateChannel(snapshot, telemetry::MetricId::HumidityRH_SCD30, now, staleAfter);

        const SensorStatus co2Status =
            EvaluateChannel(snapshot, telemetry::MetricId::CO2ppm, now, staleAfter);

        //if any core channel is offline, the whole sensor is offline
        if (tempStatus == SensorStatus::Offline ||
            humidityStatus == SensorStatus::Offline ||
            co2Status == SensorStatus::Offline) {
            return SensorStatus::Offline;
        }

        //if any core channel is invalid, mark the sensor invalid
        if (tempStatus == SensorStatus::Invalid ||
            humidityStatus == SensorStatus::Invalid ||
            co2Status == SensorStatus::Invalid) {
            return SensorStatus::Invalid;
        }

        //if any core channel is stale, mark the sensor stale
        if (tempStatus == SensorStatus::Stale ||
            humidityStatus == SensorStatus::Stale ||
            co2Status == SensorStatus::Stale) {
            return SensorStatus::Stale;
        }

        //check for disagreement with the BME680 on duplicated channels
        const auto tempFusion = fusion::SensorFusion::FuseTemperature(snapshot);
        const auto humidityFusion = fusion::SensorFusion::FuseHumidity(snapshot);

        if (tempFusion.mismatch || humidityFusion.mismatch) {
            return SensorStatus::Mismatch;
        }

        return SensorStatus::Ok;
    }

    SystemStatus SensorHealthMonitor::EvaluateSystemStatus(
        SensorStatus bme680Status,
        SensorStatus scd30Status
    ) {
        // Any offline sensor is a critical system issue
        if (bme680Status == SensorStatus::Offline ||
            scd30Status == SensorStatus::Offline) {
            return SystemStatus::Critical;
        }

        // Invalid sensors indicate a warning-level issue
        if (bme680Status == SensorStatus::Invalid ||
            scd30Status == SensorStatus::Invalid) {
            return SystemStatus::Warning;
        }

        // Stale or mismatched sensors degrade the system
        if (bme680Status == SensorStatus::Stale ||
            scd30Status == SensorStatus::Stale ||
            bme680Status == SensorStatus::Mismatch ||
            scd30Status == SensorStatus::Mismatch) {
            return SystemStatus::Degraded;
        }

        return SystemStatus::Nominal;
    }

    SensorStatus SensorHealthMonitor::EvaluateChannel(
        const telemetry::TelemetrySnapshot& snapshot,
        telemetry::MetricId metric,
        const core::time::TimePoint& now,
        core::time::Duration staleAfter
    ) {
        const auto& state = snapshot.Get(metric);

        // Never received anything for this channel.
        if (!state.value.has_value() || !state.lastUpdate.has_value()) {
            return SensorStatus::Offline;
        }

        // Latest reading was explicitly marked invalid.
        if (!state.valid) {
            return SensorStatus::Invalid;
        }

        // Reading exists but is too old.
        if ((now - *state.lastUpdate) > staleAfter) {
            return SensorStatus::Stale;
        }

        return SensorStatus::Ok;
    }

}