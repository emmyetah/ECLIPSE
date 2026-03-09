#include "AlertFormatter.h"

#include "../../telemetry/MetricSpec.h" // needed to get the metric spec

namespace eclipse::logic::alerts {

    std::string AlertFormatter::Format(const Alert& alert)
    {   
        //create an empty string 
        std::string text;

        //add severity first
        text += SeverityToString(alert.severity);
        text += ": ";

        //if this alert is tied to a metric, include the metric name
        if (alert.metric.has_value()) {
            const auto& spec = telemetry::GetMetricSpec(*alert.metric);
            text += spec.displayName;
            text += " - ";
        }

        //use the stored message if one was provided
        if (!alert.message.empty()) {
            text += alert.message;
        }
        else {
            //fallback to the alert type text if no custom message exists
            text += TypeToString(alert.type);
        }

        return text;
    }

    //takes the severity and returns a string
    const char* AlertFormatter::SeverityToString(AlertSeverity severity)
    {
        switch (severity) {
        case AlertSeverity::Info:
            return "Info";

        case AlertSeverity::Warning:
            return "Warning";

        case AlertSeverity::Critical:
            return "Critical";

        default:
            return "Unknown";
        }
    }

    //Turns the alert type to a string
    const char* AlertFormatter::TypeToString(AlertType type)
    {
        switch (type) {
        case AlertType::ThresholdBreach:
            return "Threshold breach";

        case AlertType::SensorStale:
            return "Sensor stale";

        case AlertType::SensorOffline:
            return "Sensor offline";

        case AlertType::SensorMismatch:
            return "Sensor mismatch";

        case AlertType::InvalidReading:
            return "Invalid reading";

        case AlertType::SystemDegraded:
            return "System degraded";

        case AlertType::ModeChanged:
            return "Mode changed";

        default:
            return "Unknown alert";
        }
    }
    //turns the alerts state to a string
    const char* AlertFormatter::StateToString(AlertState state)
    {
        switch (state) {
        case AlertState::Active:
            return "Active";

        case AlertState::Acknowledged:
            return "Acknowledged";

        case AlertState::Cleared:
            return "Cleared";

        default:
            return "Unknown";
        }
    }

}