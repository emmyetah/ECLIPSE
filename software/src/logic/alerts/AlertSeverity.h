#pragma once

#include <cstdint>

namespace eclipse::logic::alerts {

    //indicates how severe an alert is. Devided similar to NASA's ECLSS system
    enum class AlertSeverity : uint8_t {
        Unknown = 0, // Severity not determined

        //System alerts
        Sensor, //for sensor-down alerts

        //hazard alerts fr thresholds
        Caution,     //close to hitting threshold
        Warning,     //hit threshold but not severyly (depending on the metric. If radiaiotn hit threshold, immediate warning)
        Critical     //dangerous condition, gone way past the threshold.
    };

} 