#pragma once

#include <cstddef>
#include <vector>

#include "../telemetry/MetricId.h"
#include "../telemetry/TelemetrySnapshot.h"
#include "../telemetry/history/MetricHistory.h"

#include "../logic/TelemetryLogic.h"
#include "../logic/alerts/Alert.h"
#include "../logic/health/SystemStatus.h"
#include "../logic/health/SensorStatus.h"

#include "KpiCardVm.h"
#include "TrendPlotVm.h"

namespace eclipse::viewmodel {

    //coordinates KPI cards, trend plot, and forwards alert/system state into UI-friendly access points
    class DashboardVm {
    public:
        static constexpr std::size_t TrendHistoryCapacity = TrendPlotVm::HistoryCapacity;
        using TrendHistory = telemetry::history::MetricHistory<TrendHistoryCapacity>;

        //constructor 
        DashboardVm();

        //refreshes all child viewmodels using latest telemetry and logic state
        void Update(
            const telemetry::TelemetrySnapshot& snapshot,
            const logic::TelemetryLogic& logic,
            const TrendHistory& selectedMetricHistory
        );

        //KPI cards
        const KpiCardVm& GetTempCard() const;
        const KpiCardVm& GetHumidityCard() const;
        const KpiCardVm& GetPressureCard() const;
        const KpiCardVm& GetCo2Card() const;
        const KpiCardVm& GetRadiationCard() const;

        //Trend plot 
        const TrendPlotVm& GetTrendPlot() const;

        //select which metric is currently shown in the plot
        void SetSelectedTrendMetric(telemetry::MetricId metric);
        telemetry::MetricId GetSelectedTrendMetric() const;

        //Forwarded logic state
        const std::vector<logic::alerts::Alert>& GetAlerts() const;
        logic::health::SystemStatus GetSystemStatus() const;
        logic::health::SensorStatus GetBME680Status() const;
        logic::health::SensorStatus GetSCD30Status() const;

    private:
        //KPI cards
        KpiCardVm tempCard_;
        KpiCardVm humidityCard_;
        KpiCardVm pressureCard_;
        KpiCardVm co2Card_;
        KpiCardVm radiationCard_;

        //trend plot state
        TrendPlotVm trendPlot_;
        telemetry::MetricId selectedTrendMetric_ = telemetry::MetricId::TempC;

        //cached forwarded logic state for UI access
        std::vector<logic::alerts::Alert> alerts_;
        logic::health::SystemStatus systemStatus_ = logic::health::SystemStatus::Unknown;
        logic::health::SensorStatus bme680Status_ = logic::health::SensorStatus::Unknown;
        logic::health::SensorStatus scd30Status_ = logic::health::SensorStatus::Unknown;
    };

}