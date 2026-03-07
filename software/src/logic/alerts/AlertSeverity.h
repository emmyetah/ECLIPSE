#pragma once

#include <cstdint>

namespace eclipse::logic::alerts {

    //indicates how severe an alert is. Devided similar to NASA's ECLSS system
    enum class AlertSeverity : uint8_t {
        Unknown = 0, // Severity not determined

        Information, //e.g., Mode switch

        Warning,     //abnormal but not critical
        Critical     //dangerous condition, requiring attention
    };

} 