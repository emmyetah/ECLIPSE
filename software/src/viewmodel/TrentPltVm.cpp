#include "TrendPlotVm.h"

#include "../telemetry/MetricSpec.h"

namespace eclipse::viewmodel {

    //constructor
    TrendPlotVm::TrendPlotVm(telemetry::MetricId metric) {
        //loads the metric & metric spec
        metric_ = metric;
        const telemetry::MetricSpec& spec = telemetry::GetMetricSpec(metric_);
        label_ = std::string(spec.displayName);
        displayMin_ = spec.displayMin;
        displayMax_ = spec.displayMax;
    }

    //update plot data from metric history
    void TrendPlotVm::Update(const History& history) {
        const telemetry::MetricSpec& spec = telemetry::GetMetricSpec(metric_);

        //refresh static display metadata
        label_ = std::string(spec.displayName);
        displayMin_ = spec.displayMin;
        displayMax_ = spec.displayMax;

        //copy latest history window into VM
        history_ = history;

        //compute optional min/max/avg stats
        stats_ = telemetry::history::computeStats(history_);
    }

    //which metric this plot represents
    telemetry::MetricId TrendPlotVm::GetMetric() const {
        return metric_;
    }

    //display name for chart title
    const std::string& TrendPlotVm::GetLabel() const {
        return label_;
    }

    //chart display bounds
    double TrendPlotVm::GetDisplayMin() const {
        return displayMin_;
    }

    double TrendPlotVm::GetDisplayMax() const {
        return displayMax_;
    }

    //history data for plotting
    const TrendPlotVm::History& TrendPlotVm::GetHistory() const {
        return history_;
    }

    //optional statistics (min/max/avg)
    std::optional<TrendPlotVm::Stats> TrendPlotVm::GetStats() const {
        return stats_;
    }

}