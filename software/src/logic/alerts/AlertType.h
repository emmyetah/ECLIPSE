#pragma once

#include <cstdint>

namespace eclipse::logic::alerts {

    
    enum class AlertType : uint8_t {

        Unknown = 0,       //alert cause not determined

        //most importanrt
        ThresholdBreach,   //metric exceeded configured threshold

        SensorOffline,     //sensor disconnected or not responding
        SensorMismatch,    //sensor fusion logic shows defference too large   
        SystemHealth,      //low battery or multipe warnings combine
        ModeChanged        //system switched between Earth / Space modes
    };

} 