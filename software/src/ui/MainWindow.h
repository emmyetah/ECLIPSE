#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QButtonGroup>

#include <memory>
#include <optional>
#include <array>
#include <set>
#include <chrono>

#include "../config/Config.h"
#include "../io/parsing/TelemetryParser.h"
#include "../logic/TelemetryLogic.h"
#include "../telemetry/TelemetrySnapshot.h"
#include "../viewmodel/DashboardVm.h"
#include "../logic/fusion/SensorFusion.h"

#include "widgets/AlertsTableWidget.h"
#include "dialogs/AlertDialog.h"
#include "../logic/alerts/Alert.h"

#include "../io/serial/SerialTelemetrySource.h"
#include "../io/sim/SimTelemetrySource.h"

#include "widgets/KpiCardWidget.h"
#include "../telemetry/history/TrendWindowState.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class TrendPlotWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    struct AlertPopupKey
    {
        eclipse::logic::alerts::AlertType type =
            eclipse::logic::alerts::AlertType::Unknown;

        eclipse::logic::alerts::AlertSeverity severity =
            eclipse::logic::alerts::AlertSeverity::Unknown;

        std::optional<eclipse::telemetry::MetricId> metric;

        bool operator==(const AlertPopupKey& other) const
        {
            return type == other.type
                && severity == other.severity
                && metric == other.metric;
        }

        //Friend free function — avoids 'this' ambiguity with QWidget::metric
        friend bool operator<(const AlertPopupKey& a, const AlertPopupKey& b)
        {
            const auto t1 = static_cast<int>(a.type);
            const auto t2 = static_cast<int>(b.type);
            if (t1 != t2) return t1 < t2;

            const auto s1 = static_cast<int>(a.severity);
            const auto s2 = static_cast<int>(b.severity);
            if (s1 != s2) return s1 < s2;

            if (!a.metric.has_value() && !b.metric.has_value()) return false;
            if (!a.metric.has_value()) return true;
            if (!b.metric.has_value()) return false;
            return static_cast<int>(*a.metric) < static_cast<int>(*b.metric);
        }
    };

private:
    Ui::MainWindow* ui;
    TrendPlotWidget* trendPlot_ = nullptr;

    
    QVector<double> ConvertTrendHistory(
        const eclipse::viewmodel::DashboardVm::TrendHistory& history
    ) const;

    //application config
    eclipse::AppConfig config_ = eclipse::AppConfig::defaults();

    //telemetry pipeline
    eclipse::telemetry::TelemetrySnapshot snapshot_;
  
    eclipse::logic::TelemetryLogic logic_;
    eclipse::viewmodel::DashboardVm dashboardVm_;
    eclipse::io::parsing::TelemetryParser parser_;

    //telemetry sources
    std::unique_ptr<eclipse::io::SerialTelemetrySource> serialSource_;
    std::unique_ptr<eclipse::io::SimTelemetrySource> simSource_;
    std::unique_ptr<eclipse::io::SimTelemetrySource> spaceSimSource_;

    //trend history for selected metric - updated to store data for every metric continuosly
    std::array<eclipse::viewmodel::DashboardVm::TrendHistory,static_cast<std::size_t>(eclipse::telemetry::MetricId::Count)> metricHistories_{};
    eclipse::telemetry::history::TrendWindow selectedTrendWindow_ = eclipse::telemetry::history::TrendWindow::FiveMinutes;
    QButtonGroup* trendWindowGroup_ = nullptr;

    //update timer
    QTimer* telemetryTimer_ = nullptr;

    //KPI card wrappers
    eclipse::ui::widgets::KpiCardWidget tempCardWidget_;
    eclipse::ui::widgets::KpiCardWidget humidityCardWidget_;
    eclipse::ui::widgets::KpiCardWidget pressureCardWidget_;
    eclipse::ui::widgets::KpiCardWidget co2CardWidget_;
    eclipse::ui::widgets::KpiCardWidget radiationCardWidget_;
    eclipse::ui::widgets::AlertsTableWidget alertsTableWidget_;

    //alert dialog state
    bool alertDialogOpen_ = false;
    std::optional<AlertPopupKey> activePopupKey_;
    std::set<AlertPopupKey> shownPopupKeys_;
    std::optional<std::size_t> activePopupAlertIndex_;
    std::optional<std::size_t> lastShownPopupAlertIndex_;
    std::map<AlertPopupKey, std::chrono::steady_clock::time_point> popupKeyResolvedAt_;

    //functions  
    eclipse::telemetry::MetricId MetricFromComboIndex(int index) const;
    void PollTelemetry();
    void RefreshUi();
    void SetupTrendPlot();
    void BindKpiCards();
    void ApplyFusedMetricsToSnapshot();
    void SetupModeUI();
    void setStatusDots();
    void SetupTrendWindowButtons();
    void SetupMetricSelector();

    //alert popup helpers
    std::optional<eclipse::logic::alerts::Alert> FindPopupCandidate() const;
    AlertPopupKey MakePopupKey(const eclipse::logic::alerts::Alert& alert) const;
    void MaybeShowAlertDialog();
    void ShowAlertDialog(const eclipse::logic::alerts::Alert& alert);
    void AcknowledgePopupAlert();
    void FocusAlertsTable();


    QString FormatAlertMetric(const eclipse::logic::alerts::Alert& alert) const;
    QString FormatAlertValueNumber(const eclipse::logic::alerts::Alert& alert) const;
    QString FormatAlertValueUnit(const eclipse::logic::alerts::Alert& alert) const;
    QString FormatAlertTimestamp(const eclipse::logic::alerts::Alert& alert) const;

    //graph helpers
    eclipse::viewmodel::DashboardVm::TrendHistory& HistoryForMetric(eclipse::telemetry::MetricId metric);
    eclipse::viewmodel::DashboardVm::TrendHistory BuildFilteredHistory(const eclipse::viewmodel::DashboardVm::TrendHistory& source) const;
};
#endif