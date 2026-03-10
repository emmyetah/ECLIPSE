#pragma once

#include <string>
#include <optional>

#include "../telemetry/MetricId.h"
#include "../telemetry/MetricSpec.h"

#include "../telemetry/history/MetricHistory.h"
#include "../telemetry/history/HistoryStats.h"

namespace eclipse::viewmodel {

    //view model responsible for preparing time-series data for the dashboard trend plot
    class TrendPlotVm {
    public:

        //how many samples we keep for plotting
        static constexpr std::size_t HistoryCapacity = 300;

        using History = telemetry::history::MetricHistory<HistoryCapacity>;
        using Stats = telemetry::history::HistoryStats;

        //constructor
        explicit TrendPlotVm(telemetry::MetricId metric);

        //update plot data from metric history
        void Update(const History& history);

        //which metric this plot represents
        telemetry::MetricId GetMetric() const;

        //display name for chart title
        const std::string& GetLabel() const;

        //chart display bounds
        double GetDisplayMin() const;
        double GetDisplayMax() const;

        //history data for plotting
        const History& GetHistory() const;

        //optional statistics (min/max/avg)
        std::optional<Stats> GetStats() const;

    private:

        telemetry::MetricId metric_;

        std::string label_;

        double displayMin_ = 0.0;
        double displayMax_ = 0.0;

        History history_;

        std::optional<Stats> stats_;
    };

}