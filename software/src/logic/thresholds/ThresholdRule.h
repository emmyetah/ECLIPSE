#pragma once

#include "../../telemetry/MetricId.h"

namespace eclipse::logic::thresholds {

    //inclusive numeric range used by threshold bands.
    struct ThresholdRange {
        double min = 0.0; //lower bound of the range
        double max = 0.0; //upper bound of the range
    };

    //defines all threshold bands for a single metric
    //the evaluator will use these ranges to determine the current threshold level
    struct ThresholdRule {
        telemetry::MetricId metric = telemetry::MetricId::invalid; //default metric unless changed.

        ThresholdRange normal{};   //safe operating range
        ThresholdRange caution{};  //slightly abnormal but not serious
        ThresholdRange warning{};  //outside recommended bounds
        ThresholdRange critical{}; //dangerous range
    };

}