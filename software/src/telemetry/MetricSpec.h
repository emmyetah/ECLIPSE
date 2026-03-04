#pragma once

#include <cstdint>
#include <string_view>

#include "MetricId.h"
#include "../core/types/Units.h"

namespace eclipse::telemetry {

    // Static metadata describing how a metric should be displayed/interpreted.
    struct MetricSpec {
        MetricId id = MetricId::invalid;

        // Human-readable label for UI.
        // Example: "Temperature", "CO2", "Cabin Pressure"
        std::string_view displayName{};

        // Unit used for display and formatting.
        core::units::Unit unit = core::units::Unit::none;

        // Suggested number of decimal places when formatting for UI.
        // Example: temperature -> 2, pressure -> 1, CO2 -> 0
        uint8_t decimals = 0;

        // Expected bounds for UI normalisation (progress bars, chart ranges).
        // =they are display bounds.
        double displayMin = 0.0;
        double displayMax = 0.0;

        // Optional: UI ordering / grouping (helps build KPI rows cleanly).
        // Lower numbers appear earlier.
        uint8_t sortOrder = 0;
    };

    // Returns the static specification for a given metric.
    // If the metric is unknown/invalid, returns a safe "invalid" spec.
    const MetricSpec& GetMetricSpec(MetricId id);

}
