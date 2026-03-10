
#include "TelemetryLogic.h"

#include "fusion/SensorFusion.h"

#include "thresholds/ThresholdEvaluator.h"

#include "health/SensorHealthMonitor.h"

#include "alerts/AlertDuplicator.h"
#include "alerts/AlertFormatter.h"
#include "alerts/AlertSeverity.h"
#include "alerts/AlertState.h"
#include "alerts/AlertType.h"

#include <chrono>

namespace eclipse::logic {

    TelemetryLogic::TelemetryLogic()
        //constructor loads the earth and psace thresholds
        : earthThresholds_(mode::Mode::Earth), spaceThresholds_(mode::Mode::Space)
    {
        //sets every threshold level to unknown until we update them
        levels_.fill(thresholds::ThresholdLevel::Unknown);
    }

    void TelemetryLogic::SetMode(mode::Mode mode)
    {
        //sets the curent mode as mode given as an argument
        mode_ = mode;
    }

    mode::Mode TelemetryLogic::GetMode() const
    {
        return mode_;
    }

    void TelemetryLogic::SetEarthThresholds(const thresholds::ThresholdSet& set)
    {
        earthThresholds_ = set;
    }

    void TelemetryLogic::SetSpaceThresholds(const thresholds::ThresholdSet& set)
    {
        spaceThresholds_ = set;
    }

    void TelemetryLogic::Update(
        const telemetry::TelemetrySnapshot& snapshot,
        const core::time::TimePoint& now
    ) {
        //reset per-update state that gets recalculated from scratch
        levels_.fill(thresholds::ThresholdLevel::Unknown);

        //evaluate fused metric thresholds
        EvaluateThresholds(snapshot, now);

        //evaluate sensor and system health
        EvaluateHealth(snapshot, now);
    }

    thresholds::ThresholdLevel TelemetryLogic::GetThresholdLevel(telemetry::MetricId metric) const
    {
        if (metric == telemetry::MetricId::invalid ||
            metric == telemetry::MetricId::Count) {
            return thresholds::ThresholdLevel::Unknown;
        }
        //returns threshold levels
        return levels_[static_cast<std::size_t>(metric)];
    }

    health::SensorStatus TelemetryLogic::GetBME680Status() const
    {
        return bme680Status_;
    }

    health::SensorStatus TelemetryLogic::GetSCD30Status() const
    {
        return scd30Status_;
    }

    health::SystemStatus TelemetryLogic::GetSystemStatus() const
    {
        return systemStatus_;
    }

    const std::vector<alerts::Alert>& TelemetryLogic::GetAlerts() const
    {
        return alerts_;
    }

    void TelemetryLogic::ClearAlerts()
    {
        alerts_.clear();
    }

    const thresholds::ThresholdSet& TelemetryLogic::ActiveThresholdSet() const
    {
        return (mode_ == mode::Mode::Earth)
            ? earthThresholds_
            : spaceThresholds_;
    }

    //evaluates thresholds and sends an alert ot the alerts vector if any major threshold levels have been passed.
    void TelemetryLogic::EvaluateThresholds(
        //takes current values as argument
        const telemetry::TelemetrySnapshot& snapshot,
        const core::time::TimePoint& now
    ) {
        //sets the current thresholds
        const auto& activeSet = ActiveThresholdSet();
        {
            //fuse temperature sensors
            const auto fusionResult = fusion::SensorFusion::FuseTemperature(snapshot);
            
            if (fusionResult.value.has_value()) {
                const auto* rule = activeSet.GetRule(telemetry::MetricId::TempC);
                //set and check the current rule pointer relating to a threshold
                if (rule != nullptr) {
                    //if the rule is not empty set the thresholds
                    const auto level = thresholds::ThresholdEvaluator::Evaluate(*fusionResult.value, *rule);
                    //store the levels
                    levels_[static_cast<std::size_t>(telemetry::MetricId::TempC)] = level;
                    //send an alert if the level is warning or critical
                    if (level == thresholds::ThresholdLevel::Warning ||
                        level == thresholds::ThresholdLevel::Critical) {
                        //create a new alert, set the type, severiyy, message and itmestamp
                        alerts::Alert alert;
                        alert.type = alerts::AlertType::ThresholdBreach;
                        alert.severity = (level == thresholds::ThresholdLevel::Critical)
                            ? alerts::AlertSeverity::Critical
                            : alerts::AlertSeverity::Warning;
                        alert.state = alerts::AlertState::Active;
                        alert.metric = telemetry::MetricId::TempC;
                        alert.message = "Temperature exceeded threshold";
                        alert.timestamp = now;

                        //add the alert to the alerts vector.
                        PushAlert(alert);
                    }
                }
            }
        }

        {
            //fuse humidity sensors. Same logic as temperature
            const auto fusionResult = fusion::SensorFusion::FuseHumidity(snapshot);

            if (fusionResult.value.has_value()) {
                const auto* rule = activeSet.GetRule(telemetry::MetricId::HumidityRH);

                if (rule != nullptr) {
                    const auto level =
                        thresholds::ThresholdEvaluator::Evaluate(*fusionResult.value, *rule);

                    levels_[static_cast<std::size_t>(telemetry::MetricId::HumidityRH)] = level;

                    if (level == thresholds::ThresholdLevel::Warning ||
                        level == thresholds::ThresholdLevel::Critical) {
                        alerts::Alert alert;
                        alert.type = alerts::AlertType::ThresholdBreach;
                        alert.severity = (level == thresholds::ThresholdLevel::Critical)
                            ? alerts::AlertSeverity::Critical
                            : alerts::AlertSeverity::Warning;
                        alert.state = alerts::AlertState::Active;
                        alert.metric = telemetry::MetricId::HumidityRH;
                        alert.message = "Humidity exceeded threshold";
                        alert.timestamp = now;

                        PushAlert(alert);
                    }
                }
            }
        }

        // ----- Pressure -----
        {
            const auto value = snapshot.Value(telemetry::MetricId::PressureHpa);

            if (value.has_value()) {
                const auto* rule = activeSet.GetRule(telemetry::MetricId::PressureHpa);

                if (rule != nullptr) {
                    const auto level =
                        thresholds::ThresholdEvaluator::Evaluate(*value, *rule);

                    levels_[static_cast<std::size_t>(telemetry::MetricId::PressureHpa)] = level;

                    if (level == thresholds::ThresholdLevel::Warning ||
                        level == thresholds::ThresholdLevel::Critical) {
                        alerts::Alert alert;
                        alert.type = alerts::AlertType::ThresholdBreach;
                        alert.severity = (level == thresholds::ThresholdLevel::Critical)
                            ? alerts::AlertSeverity::Critical
                            : alerts::AlertSeverity::Warning;
                        alert.state = alerts::AlertState::Active;
                        alert.metric = telemetry::MetricId::PressureHpa;
                        alert.message = "Pressure exceeded threshold";
                        alert.timestamp = now;

                        PushAlert(alert);
                    }
                }
            }
        }

        // ----- CO2 -----
        {
            const auto value = snapshot.Value(telemetry::MetricId::CO2ppm);

            if (value.has_value()) {
                const auto* rule = activeSet.GetRule(telemetry::MetricId::CO2ppm);

                if (rule != nullptr) {
                    const auto level =
                        thresholds::ThresholdEvaluator::Evaluate(*value, *rule);

                    levels_[static_cast<std::size_t>(telemetry::MetricId::CO2ppm)] = level;

                    if (level == thresholds::ThresholdLevel::Warning ||
                        level == thresholds::ThresholdLevel::Critical) {
                        alerts::Alert alert;
                        alert.type = alerts::AlertType::ThresholdBreach;
                        alert.severity = (level == thresholds::ThresholdLevel::Critical)
                            ? alerts::AlertSeverity::Critical
                            : alerts::AlertSeverity::Warning;
                        alert.state = alerts::AlertState::Active;
                        alert.metric = telemetry::MetricId::CO2ppm;
                        alert.message = "CO2 exceeded threshold";
                        alert.timestamp = now;

                        PushAlert(alert);
                    }
                }
            }
        }

        // ----- Radiation -----
        {
            const auto value = snapshot.Value(telemetry::MetricId::RadiationCpm);

            if (value.has_value()) {
                const auto* rule = activeSet.GetRule(telemetry::MetricId::RadiationCpm);

                if (rule != nullptr) {
                    const auto level =
                        thresholds::ThresholdEvaluator::Evaluate(*value, *rule);

                    levels_[static_cast<std::size_t>(telemetry::MetricId::RadiationCpm)] = level;

                    if (level == thresholds::ThresholdLevel::Warning ||
                        level == thresholds::ThresholdLevel::Critical) {
                        alerts::Alert alert;
                        alert.type = alerts::AlertType::ThresholdBreach;
                        alert.severity = (level == thresholds::ThresholdLevel::Critical)
                            ? alerts::AlertSeverity::Critical
                            : alerts::AlertSeverity::Warning;
                        alert.state = alerts::AlertState::Active;
                        alert.metric = telemetry::MetricId::RadiationCpm;
                        alert.message = "Radiation exceeded threshold";
                        alert.timestamp = now;

                        PushAlert(alert);
                    }
                }
            }
        }
    }

    //evaluate the systems health & send alerts if needed
    void TelemetryLogic::EvaluateHealth(
        const telemetry::TelemetrySnapshot& snapshot,
        const core::time::TimePoint& now
    ) {
        //setting stale after time: if no new results in 5+ seconds then sensor is stale.
        const auto staleAfter = std::chrono::seconds(5);

        bme680Status_ = health::SensorHealthMonitor::EvaluateBME680(
            snapshot,
            now,
            staleAfter
        );

        scd30Status_ = health::SensorHealthMonitor::EvaluateSCD30(
            snapshot,
            now,
            staleAfter
        );

        systemStatus_ = health::SensorHealthMonitor::EvaluateSystemStatus(
            bme680Status_,
            scd30Status_
        );

        //alert for BME680 issues
        if (bme680Status_ == health::SensorStatus::Offline ||
            bme680Status_ == health::SensorStatus::Invalid ||
            bme680Status_ == health::SensorStatus::Stale ||
            bme680Status_ == health::SensorStatus::Mismatch) {
            //create and push alert
            alerts::Alert alert;
            alert.type = (bme680Status_ == health::SensorStatus::Mismatch)
                ? alerts::AlertType::SensorMismatch
                : (bme680Status_ == health::SensorStatus::Stale)
                ? alerts::AlertType::SensorStale
                : (bme680Status_ == health::SensorStatus::Invalid)
                ? alerts::AlertType::InvalidReading
                : alerts::AlertType::SensorOffline;

            alert.severity = (bme680Status_ == health::SensorStatus::Offline)
                ? alerts::AlertSeverity::Critical
                : alerts::AlertSeverity::Warning;

            alert.state = alerts::AlertState::Active;
            alert.message = "BME680 health issue detected";
            alert.timestamp = now;

            PushAlert(alert);
        }

        //alert for SCD30 issues
        if (scd30Status_ == health::SensorStatus::Offline ||
            scd30Status_ == health::SensorStatus::Invalid ||
            scd30Status_ == health::SensorStatus::Stale ||
            scd30Status_ == health::SensorStatus::Mismatch) {
            alerts::Alert alert;
            alert.type = (scd30Status_ == health::SensorStatus::Mismatch)
                ? alerts::AlertType::SensorMismatch
                : (scd30Status_ == health::SensorStatus::Stale)
                ? alerts::AlertType::SensorStale
                : (scd30Status_ == health::SensorStatus::Invalid)
                ? alerts::AlertType::InvalidReading
                : alerts::AlertType::SensorOffline;

            alert.severity = (scd30Status_ == health::SensorStatus::Offline)
                ? alerts::AlertSeverity::Critical
                : alerts::AlertSeverity::Warning;

            alert.state = alerts::AlertState::Active;
            alert.message = "SCD30 health issue detected";
            alert.timestamp = now;

            PushAlert(alert);
        }
    }
    //Evaluates if alert is a duplicate then adds it ot the main alert vector
    void TelemetryLogic::PushAlert(const alerts::Alert& alert)
    {
        if (!alerts::AlertDuplicator::IsDuplicate(alert, alerts_)) {
            alerts_.push_back(alert);
        }
    }

} 