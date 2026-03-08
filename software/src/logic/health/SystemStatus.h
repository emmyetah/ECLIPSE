#pragma once

#include <cstdint>

namespace eclipse::logic::health {

    //represents the overall health state of the monitoring system.
    //derived from sensor health, threshold violations, and system faults.

    //Helpful class for UI. This will change the colours for the System Status button on GUI
    enum class SystemStatus : uint8_t {
        Unknown = 0, //system state has not yet been evaluated - WHITE or BLACK

        Nominal,     //all sensors and metrics operating within normal bounds - GREEN
        Degraded,    //some sensors degraded or minor issues detected - YELLOW
        Warning,     //one or more metrics outside recommended limits - ORANGE
        Critical     //dangerous condition requiring immediate attention - RED
    };

} 