#include "KpiCardVm.h"

#include <algorithm> //std::clamp

#include "../core/utils/Format.h"

namespace eclipse::viewmodel {

    //construct VM for a specific telemetry metric
    KpiCardVm::KpiCardVm(telemetry::MetricId metric) {
        metric_ = metric;

        const telemetry::MetricSpec& spec = telemetry::GetMetricSpec(metric_);
        label_ = std::string(spec.displayName);

        displayMin_ = spec.displayMin;
        displayMax_ = spec.displayMax;
    }

    //updates the card with latest telemetry + logic state
    void KpiCardVm::Update(
        const telemetry::TelemetrySnapshot& snapshot,
        const logic::TelemetryLogic& logic
    ) {
        const telemetry::MetricSpec& spec = telemetry::GetMetricSpec(metric_);

        //refresh label + display bounds from static metric metadata
        label_ = std::string(spec.displayName);
        displayMin_ = spec.displayMin;
        displayMax_ = spec.displayMax;

        //pull latest threshold/severity from logic layer
        level_ = logic.GetThresholdLevel(metric_);

        //pull latest value from telemetry snapshot
        std::optional<double> value = snapshot.Value(metric_);

        //format value text using the metric's unit + default decimal precision
        core::format::FormatSpec formatSpec;
        formatSpec.decimalsOverride = static_cast<int>(spec.decimals);
        formatSpec.includeUnit = true;
        formatSpec.spaceBeforeUnit = true;
        formatSpec.trimTrailingZeros = false;

        valueText_ = core::format::Value(value, spec.unit, formatSpec);

        //build a 0.0 -> 1.0 normalized value for progress bars / fills
        normalizedValue_ = 0.0;

        if (value.has_value()) {
            double range = displayMax_ - displayMin_;

            if (range > 0.0) {
                double raw = (value.value() - displayMin_) / range;
                normalizedValue_ = std::clamp(raw, 0.0, 1.0);
            }
        }
    }

    //metric identifier
    telemetry::MetricId KpiCardVm::GetMetric() const {
        return metric_;
    }

    //display label (Temperature, CO2, etc.)
    const std::string& KpiCardVm::GetLabel() const {
        return label_;
    }

    //formatted value string (e.g. "23.4 C")
    const std::string& KpiCardVm::GetValueText() const {
        return valueText_;
    }

    //current threshold level (Normal / Warning / Critical)
    logic::thresholds::ThresholdLevel KpiCardVm::GetLevel() const {
        return level_;
    }

    //normalised value for progress bars (0.0 - 1.0)
    double KpiCardVm::GetNormalizedValue() const {
        return normalizedValue_;
    }

    //display bounds used by charts/bars
    double KpiCardVm::GetDisplayMin() const {
        return displayMin_;
    }

    double KpiCardVm::GetDisplayMax() const {
        return displayMax_;
    }

}