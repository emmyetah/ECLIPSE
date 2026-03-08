#include "ThresholdEvaluator.h"

namespace eclipse::logic::thresholds {

    //takes a value and compares it to the threshold rule.
    ThresholdLevel ThresholdEvaluator::Evaluate(
        double value,
        const ThresholdRule& rule
    ) {
        //check in order of importance

        //check critical range first
        if (value >= rule.critical.min && value <= rule.critical.max) {
            return ThresholdLevel::Critical;
        }

        //check warning range
        if (value >= rule.warning.min && value <= rule.warning.max) {
            return ThresholdLevel::Warning;
        }

        //check caution range
        if (value >= rule.caution.min && value <= rule.caution.max) {
            return ThresholdLevel::Caution;
        }

        //check normal range
        if (value >= rule.normal.min && value <= rule.normal.max) {
            return ThresholdLevel::Normal;
        }

        //if it does not fall into any defined band treat it as critical
        return ThresholdLevel::Critical;
    }

}