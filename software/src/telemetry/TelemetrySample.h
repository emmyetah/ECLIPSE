#pragma once

#include <optional>

#include "MetricId.h"
#include "../core/types/Time.h"

namespace eclipse::telemetry {

    //represents a single telemetry reading for one metric.
    
    struct TelemetrySample {
        // Which metric this sample belongs to
        MetricId metric = MetricId::invalid;

        // Numeric value of the sample
        double value = 0.0;

        // Timestamp used for ALL internal logic (durations, history, alerts)
        core::time::TimePoint timestamp{};

        // optional system timestamp for logging / persistence / exports
        std::optional<core::time::SystemClock::time_point> systemTimestamp{};
        
        bool valid = true;

        //constructors
        TelemetrySample() = default;

        //valid sample with required fields
        TelemetrySample(MetricId m, double v, const core::time::TimePoint& ts)
            : metric(m) //store the metric id.
            , value(v)  //store the numeric value
            , timestamp(ts) //store the internal timestamp
            , systemTimestamp(std::nullopt) //no wall-clock time by default
            , valid(true) 
        {
        }
        //Construct a sample with validity set.
        TelemetrySample(MetricId m, double v, const core::time::TimePoint & ts, bool isValid)
            : metric(m) //Store the metric id.
            , value(v)  //Store the numeric value.
            , timestamp(ts) //Store the internal timestamp.
            , systemTimestamp(std::nullopt) // No wall-clock time by default.
            , valid(isValid) //Use the provided validity flag.
        {
        }
        //Convenience: attach a system timestamp and return *this* for chaining.
        TelemetrySample& withSystemTimestamp(const core::time::SystemClock::time_point& sysTs)
        {
            systemTimestamp = sysTs; //store the wall-clock timestamp.
            return *this; //allow chaining: sample.withSystemTimestamp(...);
        }


    };

} 
