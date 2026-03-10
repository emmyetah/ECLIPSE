#include "DashboardVm.h"

namespace eclipse::viewmodel {

    //Constructuctor sets the metric id for each metric and sets Temp as the initial metric on the trendplot.
    DashboardVm::DashboardVm()
        : tempCard_(telemetry::MetricId::TempC),
        humidityCard_(telemetry::MetricId::HumidityRH),
        pressureCard_(telemetry::MetricId::PressureHpa),
        co2Card_(telemetry::MetricId::CO2ppm),
        radiationCard_(telemetry::MetricId::RadiationCpm),
        bmeTempCard_(telemetry::MetricId::TempC_BME680),
        scd30TempCard_(telemetry::MetricId::TempC_SCD30),
        bmeHumidityCard_(telemetry::MetricId::HumidityRH_BME680),
        scd30HumidityCard_(telemetry::MetricId::HumidityRH_SCD30),
        trendPlot_(telemetry::MetricId::TempC)
    {
    }

    //Refreshes all child viewmodels using latest telemetry + logic state.
    void DashboardVm::Update(
        const telemetry::TelemetrySnapshot& snapshot,
        const logic::TelemetryLogic& logic,
        const TrendHistory& selectedMetricHistory
    ) {
        //update KPI cards
        tempCard_.Update(snapshot, logic);
        humidityCard_.Update(snapshot, logic);
        pressureCard_.Update(snapshot, logic);
        co2Card_.Update(snapshot, logic);
        radiationCard_.Update(snapshot, logic);

        //update trend plot from the currently selected history
        trendPlot_.Update(selectedMetricHistory);

        //cache forwarded logic state for easy UI access
        alerts_ = logic.GetAlerts();
        systemStatus_ = logic.GetSystemStatus();
        bme680Status_ = logic.GetBME680Status();
        scd30Status_ = logic.GetSCD30Status();
    }

    //KPI cards 

    const KpiCardVm& DashboardVm::GetTempCard() const {
        return tempCard_;
    }

    const KpiCardVm& DashboardVm::GetHumidityCard() const {
        return humidityCard_;
    }

    const KpiCardVm& DashboardVm::GetPressureCard() const {
        return pressureCard_;
    }

    const KpiCardVm& DashboardVm::GetCo2Card() const {
        return co2Card_;
    }

    const KpiCardVm& DashboardVm::GetRadiationCard() const {
        return radiationCard_;
    }

    const KpiCardVm& DashboardVm::GetBmeTempCard() const {
        return bmeTempCard_;
    }

    const KpiCardVm& DashboardVm::GetScd30TempCard() const {
        return scd30TempCard_;
    }

    const KpiCardVm& DashboardVm::GetBmeHumidityCard() const {
        return bmeHumidityCard_;
    }

    const KpiCardVm& DashboardVm::GetScd30HumidityCard() const {
        return scd30HumidityCard_;
    }

    //Trend plot

    const TrendPlotVm& DashboardVm::GetTrendPlot() const {
        return trendPlot_;
    }

    void DashboardVm::SetSelectedTrendMetric(telemetry::MetricId metric) {
        selectedTrendMetric_ = metric;
    }

    telemetry::MetricId DashboardVm::GetSelectedTrendMetric() const {
        return selectedTrendMetric_;
    }

    //Forwarded logic state

    const std::vector<logic::alerts::Alert>& DashboardVm::GetAlerts() const {
        return alerts_;
    }

    logic::health::SystemStatus DashboardVm::GetSystemStatus() const {
        return systemStatus_;
    }

    logic::health::SensorStatus DashboardVm::GetBME680Status() const {
        return bme680Status_;
    }

    logic::health::SensorStatus DashboardVm::GetSCD30Status() const {
        return scd30Status_;
    }

}