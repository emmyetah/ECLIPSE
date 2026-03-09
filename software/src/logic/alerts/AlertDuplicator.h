#pragma once

#include <vector>

#include "Alert.h"

namespace eclipse::logic::alerts {

    //helps prevent the system from generating the same alert repeatedly
    class AlertDuplicator {
    public:
        //returns true if the candidate alert already exists in the list of active/recent alerts
        static bool IsDuplicate(
            const Alert& candidate,
            const std::vector<Alert>& existingAlerts
        );

    private:
        //compares two alerts and determines whether they should be treated as the same logical alert.
        static bool SameIdentity(
            const Alert& a,
            const Alert& b
        );
    };

}