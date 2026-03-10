#pragma once

#include <array>
#include <optional>
#include <vector>

#include "../telemetry/MetricId.h"
#include "../telemetry/TelemetrySnapshot.h"
#include "../core/types/Time.h"

#include "mode/Mode.h"

#include "thresholds/ThresholdLevel.h"
#include "thresholds/ThresholdSet.h"

#include "alerts/Alert.h"

#include "health/SensorStatus.h"
#include "health/SystemStatus.h"

namespace eclipse::logic {

    //central coordinator for telemetry interpretation

    class TelemetryLogic {
    public:
        TelemetryLogic();

        //sets the current operating mode Earth/Space
        void SetMode(mode::Mode mode);

        //returns the currently active mode
        mode::Mode GetMode() const;

        //replaces the threshold set used for Earth mode
        void SetEarthThresholds(const thresholds::ThresholdSet& set);

        //replaces the threshold set used for Space mode
        void SetSpaceThresholds(const thresholds::ThresholdSet& set);

        //runs one full logic update using the latest telemetry state
        void Update(
            const telemetry::TelemetrySnapshot& snapshot,
            const core::time::TimePoint& now
        );

        //returns the latest evaluated threshold level for a metric
        thresholds::ThresholdLevel GetThresholdLevel(telemetry::MetricId metric) const;

        //returns the current health of the BME680 sensor
        health::SensorStatus GetBME680Status() const;

        //returns the current health of the SCD30 sensor
        health::SensorStatus GetSCD30Status() const;

        //returns the current overall system status
        health::SystemStatus GetSystemStatus() const;

        //returns the current active/recent alerts
        const std::vector<alerts::Alert>& GetAlerts() const;

        //clears all stored alerts
        void ClearAlerts();

        //checks for the idex an alert in the alerts table, and sets the state as acknowledged
        void AcknowledgeAlert(std::size_t index);

        //removes all alerts with state = cleared
        void PurgeClearedAlerts();

        

    private:
        //returns the threshold set matching the current mode
        const thresholds::ThresholdSet& ActiveThresholdSet() const;

        //generates threshold alerts for fused metrics
        void EvaluateThresholds(
            const telemetry::TelemetrySnapshot& snapshot,
            const core::time::TimePoint& now
        );

        //generates health-related alerts
        void EvaluateHealth(
            const telemetry::TelemetrySnapshot& snapshot,
            const core::time::TimePoint& now
        );

        //adds an alert if it is not a duplicate
        void PushAlert(const alerts::Alert& alert);

        //checks if any active alerts are no longer needed and changes their state to cleared
        void MarkClearedAlerts();

    private:
        mode::Mode mode_ = mode::Mode::Earth; //current system mode

        thresholds::ThresholdSet earthThresholds_; //rules for Earth mode
        thresholds::ThresholdSet spaceThresholds_; //rules for Space mode

        //latest threshold level per metric
        std::array<thresholds::ThresholdLevel, 
        static_cast<std::size_t>(telemetry::MetricId::Count)> levels_{};

        health::SensorStatus bme680Status_ = health::SensorStatus::Unknown;
        health::SensorStatus scd30Status_ = health::SensorStatus::Unknown;
        health::SystemStatus systemStatus_ = health::SystemStatus::Unknown;

        std::vector<alerts::Alert> alerts_; //active/recent alerts
    };

}