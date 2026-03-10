#pragma once

#include <string>

#include "../telemetry/MetricId.h"
#include "../telemetry/MetricSpec.h"
#include "../telemetry/TelemetrySnapshot.h"

#include "../logic/TelemetryLogic.h"
#include "../logic/thresholds/ThresholdLevel.h"

namespace eclipse::viewmodel {

    //ViewModel representing a single KPI card on the dashboard
    class KpiCardVm {
    public:

        //construct VM for a specific telemetry metric
        explicit KpiCardVm(telemetry::MetricId metric);

        //updates the card with latest telemetry + logic state
        void Update(
            const telemetry::TelemetrySnapshot& snapshot,
            const logic::TelemetryLogic& logic
        );

        //metric identifier
        telemetry::MetricId GetMetric() const;

        //display label (Temperature, CO2, etc.)
        const std::string& GetLabel() const;

        //formatted value string (e.g. "23.4 C")
        const std::string& GetValueText() const;

        //current threshold level (Normal / Warning / Critical)
        logic::thresholds::ThresholdLevel GetLevel() const;

        //normalised value for progress bars (0.0 - 1.0)
        double GetNormalizedValue() const;

        //display bounds used by charts/bars
        double GetDisplayMin() const;
        double GetDisplayMax() const;

    private:

        telemetry::MetricId metric_;

        std::string label_;
        std::string valueText_;

        logic::thresholds::ThresholdLevel level_ = logic::thresholds::ThresholdLevel::Unknown;

        double normalizedValue_ = 0.0;

        double displayMin_ = 0.0;
        double displayMax_ = 0.0;
    };

}