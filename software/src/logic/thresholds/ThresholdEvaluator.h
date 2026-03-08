#pragma once

#include "ThresholdLevel.h"
#include "ThresholdRule.h"

namespace eclipse::logic::thresholds {

    //responsible for determining which threshold level
    class ThresholdEvaluator {
    public:

        //evaluates a value against the given rule and returns the corresponding threshold level
        static ThresholdLevel Evaluate(
            double value,
            const ThresholdRule& rule
        );

    };

}