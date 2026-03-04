#include "TelemetrySnapshot.h"

namespace eclipse::telemetry
{
    TelemetrySnapshot::TelemetrySnapshot()
    {
       //explicitly clear to ensure consistent reset behaviour.
        Clear();
    }

    void TelemetrySnapshot::Apply(const TelemetrySample& sample)
    {
        //ignore invalid metric ids
        if (sample.metric == MetricId::invalid || sample.metric == MetricId::Count) return;

        //convert metric id to index
        const std::size_t idx = ToIndex(sample.metric);

        //ensure index is inside array bounds
        if (idx >= states_.size()) return;

        //update lastAnyUpdate_ even if sample is invalid, because we received something might matter for UI freshness.
        lastAnyUpdate_ = sample.timestamp;

        //update this metric's lastUpdate
        states_[idx].lastUpdate = sample.timestamp;

        //update valid flag to match the sample validity
        states_[idx].valid = sample.valid;

        //if valid, store the value
        if (sample.valid)
        {
            states_[idx].value = sample.value;
        } 
        //if invalid, clear the value so UI/logic knows it's unusable.
        else
        {
            states_[idx].value.reset();
        }
    }

    const MetricState& TelemetrySnapshot::Get(MetricId id) const
    {
        //if invalid, return a safe static empty state.
        static const MetricState kEmptyState{ std::nullopt, std::nullopt, false };

        if (id == MetricId::invalid || id == MetricId::Count)
            return kEmptyState;

        const std::size_t idx = ToIndex(id);

        if (idx >= states_.size())
            return kEmptyState;

        return states_[idx];
    }

    bool TelemetrySnapshot::HasValue(MetricId id) const
    {
        //Get the metric state
        const auto& s = Get(id);

        //value exists and is marked valid
        return s.valid && s.value.has_value();
    }

    std::optional<double> TelemetrySnapshot::Value(MetricId id) const
    {
        //if there's no valid value, return empty
        if (!HasValue(id))
            return std::nullopt;

        //Return the stored value
        return Get(id).value;
    }

    //Clear every metric state.
    void TelemetrySnapshot::Clear()
    {
       
        for (auto& s : states_)
        {
            s.value.reset();      //clear numeric value
            s.lastUpdate.reset(); //clear last update time
            s.valid = false;      //mark as invalid
        }

        //Clear the "any metric" timestamp
        lastAnyUpdate_.reset();
    }

    std::optional<core::time::TimePoint> TelemetrySnapshot::LastAnyUpdate() const
    {
        //return the timestamp of the last received metric update (if any).
        return lastAnyUpdate_;
    }

}