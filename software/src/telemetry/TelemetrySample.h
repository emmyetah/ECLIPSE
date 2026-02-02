#pragma once

#include <optional>

#include "MetricId.h"
#include "../core/types/Time.h"

namespace telemetry {

    // Represents a single telemetry reading for one metric.
    // This is a raw data point, not processed or aggregated.
    struct TelemetrySample {
        // Which metric this sample belongs to
        MetricId metric = MetricId::invalid;

        // Numeric value of the sample
        double value = 0.0;

        // Timestamp used for ALL internal logic (durations, history, alerts)
        core::time::TimePoint timestamp;

        // optional system timestamp for logging / persistence / exports
        std::optional<core::time::SystemClock::time_point> systemTimestamp;
        // ^
        // | Optional: marks whether this value is considered valid
        // (useful for sensor dropouts or parsing errors)
        bool valid = true;
    };

} 
