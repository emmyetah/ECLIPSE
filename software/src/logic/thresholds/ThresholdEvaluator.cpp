#include "ThresholdEvaluator.h"

namespace eclipse::logic::thresholds {

    namespace {
        bool InRange(double value, const ThresholdRange& range) {
            return value >= range.min && value <= range.max;
        }
    }

    ThresholdLevel ThresholdEvaluator::Evaluate(
        double value,
        const ThresholdRule& rule
    ) {
        // Check most severe band first
        if (InRange(value, rule.critical)) {
            return ThresholdLevel::Critical;
        }

        if (InRange(value, rule.warning)) {
            return ThresholdLevel::Warning;
        }

        if (InRange(value, rule.caution)) {
            return ThresholdLevel::Caution;
        }

        if (InRange(value, rule.normal)) {
            return ThresholdLevel::Normal;
        }

        return ThresholdLevel::Unknown;
    }

}