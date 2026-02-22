#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include "MetricId.h"
#include "TelemetrySample.h"
#include "../core/types/Time.h"

namespace telemetry {

    // Holds the latest known state for a single metric.
    struct MetricState {
        // Latest numeric value (missing if never received or invalidated)
        std::optional<double> value;

        // When the latest value was received (steady time for logic)
        std::optional<core::time::TimePoint> lastUpdate;

        // Whether the last received value was marked valid
        bool valid = false;
    };

    // Snapshot of the latest telemetry values for all metrics at a point in time.
    // This is the "current state" backing the dashboard.
    class TelemetrySnapshot {
    public:
        TelemetrySnapshot();

        // Apply a new sample (updates the corresponding metric state)
        void Apply(const TelemetrySample& sample);

        // Returns the current state for a metric (value + timing + validity)
        const MetricState& Get(MetricId id) const;

        // Convenience helpers
        bool HasValue(MetricId id) const;
        std::optional<double> Value(MetricId id) const;

        // Clears all values (useful for resets/testing)
        void Clear();

        // When this snapshot was last updated by ANY metric
        std::optional<core::time::TimePoint> LastAnyUpdate() const;

    private:
        static constexpr std::size_t ToIndex(MetricId id) {
            return static_cast<std::size_t>(id);
        }

        std::array<MetricState, static_cast<std::size_t>(MetricId::count)> states_{};
        std::optional<core::time::TimePoint> lastAnyUpdate_{};
    };

} 
