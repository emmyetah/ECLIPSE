#include "MainWindow.h"
#include "./ui_mainwindow.h"
#include "widgets/TrendPltWidget.h"

#include <QVBoxLayout>
#include <chrono>
#include <QDebug>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "MainWindow constructor reached";
    SetupTrendPlot();

    qDebug() << "About to choose telemetry source";
    qDebug() << "Sample period ms:" << config_.samplePeriodMs;

    //choose telemetry source
    if (config_.telemetry.sourceType == eclipse::TelemetrySourceType::Serial)
    {
        serialSource_ = std::make_unique<eclipse::io::SerialTelemetrySource>(
            config_.telemetry.serial
        );
        qDebug() << "Serial source created";

        bool opened = serialSource_->open();
        qDebug() << "Serial source open result:" << opened;
        qDebug() << "Serial source isOpen():" << serialSource_->isOpen();
    }
    else
    {
        simSource_ = std::make_unique<eclipse::io::SimTelemetrySource>(
            config_.telemetry.sim
        );
        qDebug() << "Sim source created";

        simSource_->open();
        qDebug() << "Sim source open called";
    }

    //create timer
    telemetryTimer_ = new QTimer(this);

    qDebug() << "Telemetry timer created";

    connect(
        telemetryTimer_,
        &QTimer::timeout,
        this,
        &MainWindow::PollTelemetry
    );
    qDebug() << "Telemetry timer connected";

    telemetryTimer_->start(config_.samplePeriodMs);
    qDebug() << "Telemetry timer started. Active:"
        << telemetryTimer_->isActive()
        << "Interval:"
        << telemetryTimer_->interval();
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
    qDebug() << "PollTelemetry called";
    
    std::optional<std::string> line;
  
    if (serialSource_)
        line = serialSource_->pollLine();

    if (simSource_)
        line = simSource_->pollLine();

    qDebug() << "After pollLine call";
    if (line.has_value())
        qDebug() << "Polled line:" << QString::fromStdString(*line);
    else
        qDebug() << "No line returned from source";

    if (!line)
        return;

    auto parsed = parser_.parseLine(*line);

    //debug
  qDebug() << "PollTelemetry tick";

    if (line.has_value())
        qDebug() << "Line:" << QString::fromStdString(*line);
    else
        qDebug() << "No line";

    if (parsed.has_value())
        qDebug() << "Parsed samples:" << parsed->size();
    else
        qDebug() << "Parser rejected line";

    qDebug() << "Pressure value exists:" << snapshot_.Value(eclipse::telemetry::MetricId::PressureHpa).has_value();
    qDebug() << "CO2 value exists:" << snapshot_.Value(eclipse::telemetry::MetricId::CO2ppm).has_value();
    qDebug() << "Radiation value exists:" << snapshot_.Value(eclipse::telemetry::MetricId::RadiationCpm).has_value();

    if (!parsed)
        return;

    for (const auto& sample : *parsed)
    {
        snapshot_.Apply(sample);
    }

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