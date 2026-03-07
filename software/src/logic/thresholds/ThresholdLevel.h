#pragma once

#include <cstdint>

namespace eclipse::logic::thresholds {
    
    //holds the alert levels 
    enum class ThresholdLevel : uint8_t {
        Unknown = 0, // Value not yet evaluated or unavailable

        Normal,      
        Caution,     // Approaching limit
        Warning,     //outside recommended range
        Critical     //dangerous / emergency condition
    };

} 