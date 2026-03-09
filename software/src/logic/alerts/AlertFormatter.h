#pragma once

#include <string>

#include "Alert.h"

namespace eclipse::logic::alerts {

    //formats alerts into human-readable text for UI and logging
    class AlertFormatter {
    public:
        //builds a full alert message
        static std::string Format(const Alert& alert);

        //converts severity to display text
        static const char* SeverityToString(AlertSeverity severity);

        //converts alert type to display text
        static const char* TypeToString(AlertType type);

        //converts alert state to display text
        static const char* StateToString(AlertState state);
    };

}