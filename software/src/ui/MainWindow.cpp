#include "MainWindow.h"
#include "./ui_mainwindow.h"
#include "widgets/TrendPltWidget.h"

#include <QVBoxLayout>
#include <chrono>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SetupTrendPlot();

    //choose telemetry source
    if (config_.telemetry.sourceType == eclipse::TelemetrySourceType::Serial)
    {
        serialSource_ = std::make_unique<eclipse::io::SerialTelemetrySource>(
            config_.telemetry.serial
        );

        serialSource_->open();
    }
    else
    {
        simSource_ = std::make_unique<eclipse::io::SimTelemetrySource>(
            config_.telemetry.sim
        );

        simSource_->open();
    }

    //create timer
    telemetryTimer_ = new QTimer(this);

    connect(
        telemetryTimer_,
        &QTimer::timeout,
        this,
        &MainWindow::PollTelemetry
    );

    telemetryTimer_->start(config_.samplePeriodMs);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupTrendPlot()
{
    trendPlot_ = new TrendPlotWidget(this);

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->trendChartContainer->layout());
    if (layout == nullptr) {
        layout = new QVBoxLayout(ui->trendChartContainer);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }

    layout->addWidget(trendPlot_);
}


void MainWindow::PollTelemetry()
{
    std::optional<std::string> line;

    if (serialSource_)
        line = serialSource_->pollLine();

    if (simSource_)
        line = simSource_->pollLine();

    if (!line)
        return;

    auto parsed = parser_.parseLine(*line);

    if (!parsed)
        return;

    snapshot_ = *parsed;

    auto now = std::chrono::steady_clock::now();
    logic_.Update(snapshot_, now);

    auto selectedMetric = dashboardVm_.GetTrendPlot().GetMetric();
    auto value = snapshot_.Value(selectedMetric);

    if (value.has_value())
    {
        selectedTrendHistory_.add(now, value.value());
    }

    dashboardVm_.Update(snapshot_, logic_, selectedTrendHistory_);

    RefreshUi();
}

void MainWindow::RefreshUi()
{
    tempCardWidget_.Update(dashboardVm_.GetTempCard());
    humidityCardWidget_.Update(dashboardVm_.GetHumidityCard());
    pressureCardWidget_.Update(dashboardVm_.GetPressureCard());
    co2CardWidget_.Update(dashboardVm_.GetCo2Card());
    radiationCardWidget_.Update(dashboardVm_.GetRadiationCard());

    const auto& trendVm = dashboardVm_.GetTrendPlot();

    trendPlot_->SetMetricData(
        QString::fromStdString(trendVm.GetLabel()),
        "Value",
        ConvertTrendHistory(trendVm.GetHistory())
    );
}

QVector<double> MainWindow::ConvertTrendHistory(
    const eclipse::viewmodel::DashboardVm::TrendHistory& history
) const
{
    QVector<double> values;
    values.reserve(static_cast<int>(history.size()));

    for (std::size_t i = 0; i < history.size(); i++)
    {
        values.append(history.point(i).v);
    }

    return values;
}