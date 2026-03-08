#pragma once

#include <array>
#include <cstddef>

#include "../mode/Mode.h" // for earth & space mode
#include "ThresholdRule.h" //stores the thresholds
#include "../../telemetry/MetricId.h" //rules are indexed by metric ID

namespace eclipse::logic::thresholds {

    //holds all threshold rules for a single operating mode
    //one set for Earth mode, another for Space mode.
    class ThresholdSet {
    public:
        //constructs an empty threshold set and remember which mode it belongs to a
        explicit ThresholdSet(mode::Mode mode)
            : mode_(mode)
        {
        }

        //funciton returns the operating mode this threshold set belongs to.
        mode::Mode mode() const {
            return mode_;
        }

        //Sets a rule. If a rule doesn't exsist adds it and if it already exsists overwrites it 
        void SetRule(const ThresholdRule& rule) {
            const std::size_t idx = static_cast<std::size_t>(rule.metric);

            //ignore invalidl metric ids
            if (rule.metric == telemetry::MetricId::invalid ||
                rule.metric == telemetry::MetricId::Count)
                return;
            //sets rule
            rules_[idx] = rule;
            hasRule_[idx] = true;
        }

        //Returns a pointer because pointers can be null & doesn't modify the threshold set
        // --
        //Returns the rule for a metric, or nullptr if no rule exists.
        const ThresholdRule* GetRule(telemetry::MetricId metric) const {
            //check metric id
            if (metric == telemetry::MetricId::invalid ||
                metric == telemetry::MetricId::Count)
                return nullptr;

            //convert metric to index
            const std::size_t idx = static_cast<std::size_t>(metric);

            //check if a rule has been assigned ot that index already, if not return nullptr
            if (!hasRule_[idx])
                return nullptr;
            //otherwise return the pointer to the stored rule.
            return &rules_[idx];
        }

        //returns true if the set contains a rule for the metric.
        bool HasRule(telemetry::MetricId metric) const {
            return GetRule(metric) != nullptr;
        }

    private:
        //defaulting ot earth because it gives the member a safe default case
        mode::Mode mode_ = mode::Mode::Earth; //mode this rule set belongs to

        //used array as it's fixed size, fast and no heap allocation
        //creates a fixed size array of Threshold rules, the size is the numbe rof metric IDs
        std::array<ThresholdRule, static_cast<std::size_t>(telemetry::MetricId::Count)> rules_{};

        //an array that helps decide what metrics have rules, if true they can be stored inside rules_
        //tracks whether a real rule has been assigned for each metric.
        std::array<bool, static_cast<std::size_t>(telemetry::MetricId::Count)> hasRule_{};
    };

} 