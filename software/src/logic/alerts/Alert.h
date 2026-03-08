#pragma once

#include <optional>
#include <string>

#include "AlertSeverity.h"
#include "AlertState.h"
#include "AlertType.h"
#include "../../telemetry/MetricId.h"
#include "../../core/types/Time.h"

namespace eclipse::logic::alerts {

    //represents a single alert in the system.
    struct Alert {
        AlertType type = AlertType::Unknown; //what caused the alert
        AlertSeverity severity = AlertSeverity::Unknown; //how serious the alert is
        AlertState state = AlertState::Unknown; //current lifecycle state of the alert

        //metric associated with the alert if applicable
        std::optional<telemetry::MetricId> metric;

        //human-readable alert message for UI/logging.
        std::string message{};

        //timestamp for when the alert was created or updated.
        core::time::TimePoint timestamp{};
    };

}