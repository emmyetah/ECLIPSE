#pragma once

#include <cstdint>

namespace eclipse::logic::alerts {

    //represents the lifecycle state of an alert.
    enum class AlertState : uint8_t {
        Unknown = 0,   // State not yet determined

        Active,        //alert condition currently present
        Acknowledged,  //user has acknowledged the alert
        Cleared        //condition returned to normal
    };

}