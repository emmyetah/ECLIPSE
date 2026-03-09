#include "AlertDuplicator.h"

namespace eclipse::logic::alerts {

    bool AlertDuplicator::IsDuplicate(
        const Alert& candidate,
        const std::vector<Alert>& existingAlerts
    ) {
        //compare the alert against every existing alert
        for (const auto& alert : existingAlerts) {
            if (SameIdentity(candidate, alert)) {
                return true;
            }
        }

        return false;
    }

    bool AlertDuplicator::SameIdentity(
        const Alert& a,
        const Alert& b
    ) {
        //of any details do not match then the alert is not the same
         
        //alert type must match
        if (a.type != b.type) {
            return false;
        }

        //severity must match
        if (a.severity != b.severity) {
            return false;
        }

        //metric association must match
        if (a.metric != b.metric) {
            return false;
        }

        return true;
    }

} 