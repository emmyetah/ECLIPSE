#pragma once

#include <cstdint>

namespace eclipse::logic::health {

    //represents the current health state of an individual sensor
    enum class SensorStatus : uint8_t {
        Unknown = 0, //sensor has not yet been evaluated

        Ok,          //sensor is operating normally
        Stale,       //sensor has stopped updating within the expected time frame
        Offline,     //sensor is disconnected or unavailable
        Mismatch     //sensor disagrees with another sensor beyond tolerance e.g., temp_BME = 56 temp_SCD =12
    };

}