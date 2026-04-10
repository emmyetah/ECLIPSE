#include "MainWindow.h"
#include "./ui_mainwindow.h"
#include "widgets/TrendPltWidget.h"
#include "../core/types/Time.h"

#include "dialogs/AlertDialog.h"
#include "../logic/alerts/AlertFormatter.h"
#include "../telemetry/MetricSpec.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <chrono>
#include <QDebug>
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SetupModeUI();
    BindKpiCards();
    SetupTrendPlot();
    SetupMetricSelector();
    SetupTrendWindowButtons();
    alertsTableWidget_.Bind(ui->alertTable);
    
    //connecting clear alerts button in constructor as its a quick addition
    connect(ui->clearAlertsButton, &QPushButton::clicked, this, [this]() {
        alertsTableWidget_.ClearTable();
        });

    //construct sim source fallback in case of serial crash
    simSource_ = std::make_unique<eclipse::io::SimTelemetrySource>(
            config_.telemetry.sim
        );
        qDebug() << "Sim source created";

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

    serialRetryTimer_ = new QTimer(this);
    serialRetryTimer_->setInterval(5000); //probe every 5 seconds
    connect(serialRetryTimer_, &QTimer::timeout, this, [this]()
        {
            if (!simFallbackActive_ || !serialSource_)
                return;

            qDebug() << "Probing serial port...";

            if (serialSource_->isOpen())
                serialSource_->close();

            bool reopened = serialSource_->open();
            if (reopened && serialSource_->isOpen())
            {
                auto testLine = serialSource_->pollLine();
                if (!testLine.has_value())
                {
                    qDebug() << "Port opened but no data yet, staying in sim";
                    serialSource_->close();
                    return; // retry again in 5 seconds
                }

                qDebug() << "Serial restored - leaving sim fallback";
                simFallbackActive_ = false;
                lastValidTelemetryTime_ = std::chrono::steady_clock::now();
                if (simSource_ && simSource_->isOpen())
                    simSource_->close();
                serialRetryTimer_->stop();
                ui->spaceCapsulePushButton->setChecked(false);
                ui->earthModePushButton->setChecked(true);
            }
        });

    //adding sim setup to contructor
    spaceSimSource_ = std::make_unique<eclipse::io::SimTelemetrySource>(config_.spaceSim);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupTrendPlot()
{
    trendPlot_ = new TrendPlotWidget(this);

    connect(trendPlot_, &TrendPlotWidget::ExportStarted,
        this, [this]() { telemetryTimer_->stop(); });

    connect(trendPlot_, &TrendPlotWidget::ExportFinished,
        this, [this]() { telemetryTimer_->start(config_.samplePeriodMs); });

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->trendChartContainer->layout());
    if (layout == nullptr) {
        layout = new QVBoxLayout(ui->trendChartContainer);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }

    layout->addWidget(trendPlot_);
    connect(ui->exportButton, &QPushButton::clicked, trendPlot_, &TrendPlotWidget::OnExportClicked);
}

void MainWindow::setStatusDots() {

    const QString green = "#57C785";
    const QString orange = "#FF9F43";
    const QString red = "#FF5C5C";

    auto applyDotStyle = [](QWidget* dot, const QString& colour)
        {
            if (dot == nullptr) return;

            dot->setStyleSheet(
                "color:" + colour + ";"
                "border-radius:6px;"
                "border:none;"
            );
        };

    auto latestUpdateAgeMs = [this]() -> long long
        {
            std::optional<core::time::TimePoint> newest;

            const auto checkMetric = [this, &newest](eclipse::telemetry::MetricId metric)
                {
                    const auto& state = snapshot_.Get(metric);

                    if (state.lastUpdate.has_value())
                    {
                        if (!newest.has_value() || *state.lastUpdate > *newest)
                        {
                            newest = state.lastUpdate;
                        }
                    }
                };

            checkMetric(eclipse::telemetry::MetricId::TempC);
            checkMetric(eclipse::telemetry::MetricId::HumidityRH);
            checkMetric(eclipse::telemetry::MetricId::PressureHpa);
            checkMetric(eclipse::telemetry::MetricId::CO2ppm);
            checkMetric(eclipse::telemetry::MetricId::RadiationCpm);

            if (!newest.has_value())
            {
                return std::numeric_limits<long long>::max();
            }

            const auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now - *newest).count();
        };
    //LATENCY DOT
    // Green  = fresh data arriving close to expected rate
    // Orange = delayed but not yet stale
    // Red    = very delayed / effectively stalled
    {
        const long long ageMs = latestUpdateAgeMs();
        const int sampleMs = static_cast<int>(config_.samplePeriodMs);

        QString latencyColour = red;

        if (ageMs <= (sampleMs * 2))
        {
            latencyColour = green;
        }
        else if (ageMs <= 5000)
        {
            latencyColour = orange;
        }
        else
        {
            latencyColour = red;
        }

        applyDotStyle(ui->latencyStatusDot, latencyColour);
    }

    //SENSORS DOT
    // Nominal -> green
    // Degraded / Warning -> orange
    // Critical / Offline -> red
    {
        const auto systemStatus = dashboardVm_.GetSystemStatus();
        qDebug() << "System status:" << (int)dashboardVm_.GetSystemStatus();

        QString sensorsColour = red;

        if (systemStatus == eclipse::logic::health::SystemStatus::Nominal)
        {
            sensorsColour = green;
        }
        else if (systemStatus == eclipse::logic::health::SystemStatus::Degraded ||
            systemStatus == eclipse::logic::health::SystemStatus::Warning)
        {
            sensorsColour = orange;
        }
        else
        {
            sensorsColour = red;
        }

        applyDotStyle(ui->sensorStatusDot, sensorsColour);
    }

    //ALERTS DOT
    // Red    = any active critical alert
    // Orange = any active warning OR acknowledged critical/warning
    // Green  = no meaningful current alerts
    {
        const auto& alerts = dashboardVm_.GetAlerts();

        bool hasRed = false;
        bool hasOrange = false;

        for (const auto& alert : alerts)
        {
            if (alert.state == eclipse::logic::alerts::AlertState::Cleared)
            {
                continue;
            }

            if (alert.state == eclipse::logic::alerts::AlertState::Active)
            {
                if (alert.severity == eclipse::logic::alerts::AlertSeverity::Critical)
                {
                    hasRed = true;
                    break;
                }

                if (alert.severity == eclipse::logic::alerts::AlertSeverity::Warning ||
                    alert.severity == eclipse::logic::alerts::AlertSeverity::Information)
                {
                    hasOrange = true;
                }
            }
            else if (alert.state == eclipse::logic::alerts::AlertState::Acknowledged)
            {
                if (alert.severity == eclipse::logic::alerts::AlertSeverity::Critical ||
                    alert.severity == eclipse::logic::alerts::AlertSeverity::Warning)
                {
                    hasOrange = true;
                }
            }
        }

        QString alertsColour = green;

        if (hasRed)
        {
            alertsColour = red;
        }
        else if (hasOrange)
        {
            alertsColour = orange;
        }
        else
        {
            alertsColour = green;
        }

        applyDotStyle(ui->alertsStatusDot, alertsColour);
    }

    // CONNECTION DOT
    // Green  = source exists and is open
    // Red    = no source / source closed
    {
        bool connected = false;

        if (serialSource_)
        {
            connected = serialSource_->isOpen();
        }
        else if (simSource_)
        {
            connected = simSource_->isOpen();
        }

        applyDotStyle(ui->connectionStatusDot, connected ? green : red);
    }
    
}

void MainWindow::BindKpiCards() {
    tempCardWidget_.Bind(
        ui->tempKpiLabel,
        ui->tempKpiDataLabel,
        ui->tempMin,
        ui->tempMax,
        ui->tempStatusLabel,
        ui->tempTimeStamp,
        ui->tempSlider,
        ui->tempStatusBar
    );
    humidityCardWidget_.Bind(
        ui->humidityKpiLabel,
        ui->humidityKpiDataLabel,
        ui->humidityMin,
        ui->humidityMax,
        ui->humidityStatusLabel,
        ui->humidityTimeStamp,
        ui->humiditySlider,
        ui->humidityStatusBar
    );
    pressureCardWidget_.Bind(
        ui->pressureKpiLabel,
        ui->pressureKpiDataLabel,
        ui->pressureMin,
        ui->pressureMax,
        ui->pressureStatusLabel,
        ui->pressureTimeStamp,
        ui->pressureSlider,
        ui->pressureStatusBar
    );
    co2CardWidget_.Bind(
        ui->co2KpiLabel,
        ui->co2KpiDataLabel,
        ui->co2Min,
        ui->co2Max,
        ui->co2StatusLabel,
        ui->co2TimeStamp,
        ui->co2Slider,
        ui->co2StatusBar
    );
    radiationCardWidget_.Bind(
        ui->radiationKpiLabel,
        ui->radiationKpiDataLabel,
        ui->radiationMin,
        ui->radiationMax,
        ui->radiationStatusLabel,
        ui->radiationTimeStamp,
        ui->radiationSlider,
        ui->radiationStatusBar
    );
}

void MainWindow::ApplyFusedMetricsToSnapshot()
{
    auto tempFusion = eclipse::logic::fusion::SensorFusion::FuseTemperature(snapshot_);

    auto now = std::chrono::steady_clock::now();

    if (tempFusion.value.has_value()) {
        eclipse::telemetry::TelemetrySample n;
        n.metric = eclipse::telemetry::MetricId::TempC;
        n.value = *tempFusion.value;
        n.timestamp = now;
        snapshot_.Apply(n);
    }

    auto humidityFusion = eclipse::logic::fusion::SensorFusion::FuseHumidity(snapshot_);
    if (humidityFusion.value.has_value()) {
        eclipse::telemetry::TelemetrySample n;
        n.metric = eclipse::telemetry::MetricId::HumidityRH;
        n.value = *humidityFusion.value;
        n.timestamp = now;
        snapshot_.Apply(n);
    }
}

void MainWindow::SetupModeUI()
{
    ui->earthModePushButton->setCheckable(true); 
    ui->spaceCapsulePushButton->setCheckable(true); 

    //default mode = Earth
    logic_.SetMode(eclipse::logic::mode::Mode::Earth);

    //highlight Earth button
    ui->earthModePushButton->setChecked(true);
    ui->spaceCapsulePushButton->setChecked(false);

    //connect Earth button
    connect(ui->earthModePushButton, &QPushButton::clicked, this, [this]()
        {
            logic_.SetMode(eclipse::logic::mode::Mode::Earth);

            //stop space sim, start earth sim (or serial)
            if (spaceSimSource_ && spaceSimSource_->isOpen())
                spaceSimSource_->close();

            if (simSource_ && !simSource_->isOpen())
                simSource_->open();

            for (auto& history : metricHistories_)
                history.clear();

            snapshot_ = eclipse::telemetry::TelemetrySnapshot{}; //clear stale space data

            ui->earthModePushButton->setChecked(true);
            ui->spaceCapsulePushButton->setChecked(false);
    });
    //connect space buttone 
    connect(ui->spaceCapsulePushButton, &QPushButton::clicked, this, [this]()
        {
            logic_.SetMode(eclipse::logic::mode::Mode::Space);

            //stop earth sim and serial, start space sim
            if (simSource_ && simSource_->isOpen())
                simSource_->close();


            if (spaceSimSource_ && !spaceSimSource_->isOpen())
                spaceSimSource_->open();

            for (auto& history : metricHistories_)
                history.clear();

            snapshot_ = eclipse::telemetry::TelemetrySnapshot{}; //clear stale earth data

            ui->earthModePushButton->setChecked(false);
            ui->spaceCapsulePushButton->setChecked(true);
    });
}

eclipse::telemetry::MetricId MainWindow::MetricFromComboIndex(int index) const
{
    switch (index)
    {
    case 0: return eclipse::telemetry::MetricId::TempC;
    case 1: return eclipse::telemetry::MetricId::PressureHpa;
    case 2: return eclipse::telemetry::MetricId::HumidityRH;
    case 3: return eclipse::telemetry::MetricId::CO2ppm;
    case 4: return eclipse::telemetry::MetricId::RadiationCpm;
    default: return eclipse::telemetry::MetricId::TempC;
    }
}

//gets history for a given metric
eclipse::viewmodel::DashboardVm::TrendHistory& MainWindow::HistoryForMetric(
    eclipse::telemetry::MetricId metric
)
{
    return metricHistories_[static_cast<std::size_t>(metric)];
}

void MainWindow::SetupMetricSelector()
{
    ui->selectMetricBox->setCurrentIndex(0);

    connect(
        ui->selectMetricBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        [this](int index)
        {
            const auto metric = MetricFromComboIndex(index);

            dashboardVm_.SetSelectedTrendMetric(metric);
            RefreshUi();
        }
    );
}


void MainWindow::PollTelemetry()
{
    qDebug() << "PollTelemetry called";
    
    std::optional<std::string> line;
    bool lineFromSerial = false;
 
    if (logic_.GetMode() == eclipse::logic::mode::Mode::Space)
    {
        if (spaceSimSource_)
            line = spaceSimSource_->pollLine();
    }
    else
    {
        if (simFallbackActive_)
        {
            if (simSource_ && simSource_->isOpen())
                line = simSource_->pollLine();
        }
        else if (serialSource_ && serialSource_->isOpen())
        {
            line = serialSource_->pollLine();
            lineFromSerial = true;
        }

        else if (simSource_ && simSource_->isOpen())
        {
            line = simSource_->pollLine();
        }
    }

    qDebug() << "After pollLine call";
    if (line.has_value())
        qDebug() << "Polled line:" << QString::fromStdString(*line);
    else
        qDebug() << "No line returned from source";

    const auto nowTime = std::chrono::steady_clock::now();
    const auto telemetryTimeout = std::chrono::seconds(5);

    if (!line.has_value()) {
        if (!simFallbackActive_ && serialSource_ && serialSource_->isOpen()) {
            if ((nowTime - lastValidTelemetryTime_) > telemetryTimeout && !simFallbackActive_) {
                qDebug() << "Hardware telemetry timeout - switching to sim mode";

                simFallbackActive_ = true;
                lineFromSerial = false;
                serialSource_->close();
                serialRetryTimer_->start();

                qDebug() << "Entering sim fallback - updating UI";
                ui->earthModePushButton->setChecked(false);
                ui->spaceCapsulePushButton->setChecked(true);

                if (simSource_ && !simSource_->isOpen()) {
                    simSource_->open();
                }

                line = simSource_->pollLine();
            }
        }
        if (!line.has_value())
            return;
    }

    QElapsedTimer e2eTimer;
    e2eTimer.start();

    auto parsed = parser_.parseLine(*line);


    if (line.has_value())
        qDebug() << "Line:" << QString::fromStdString(*line);
    else
        qDebug() << "No line";

    if (parsed.has_value())
        qDebug() << "Parsed samples:" << parsed->size();
    else
        qDebug() << "Parser rejected line";

   
    if (!parsed)
        return;

    if (simFallbackActive_ && lineFromSerial) {
        qDebug() << "Hardware telemetry restored - switching back to serial";

        simFallbackActive_ = false;
        serialRetryTimer_->stop();
        if (simSource_ && simSource_->isOpen())
            simSource_->close();
            ui->spaceCapsulePushButton->setChecked(false); 
            ui->earthModePushButton->setChecked(true);
        
    }

    if (lineFromSerial) {
        lastValidTelemetryTime_ = std::chrono::steady_clock::now();
    }

    for (const auto& sample : *parsed)
    {
        snapshot_.Apply(sample);
    } 

    ApplyFusedMetricsToSnapshot();

    
    auto now = std::chrono::steady_clock::now();
    logic_.Update(snapshot_, now);



    //updted history logic

    //adds new metric data to snapshot 
    const auto appendMetricHistory = [this, &now](eclipse::telemetry::MetricId metric)
        {
            const auto value = snapshot_.Value(metric);

            if (value.has_value())
            {
                HistoryForMetric(metric).add(now, *value);
            }
        };

    appendMetricHistory(eclipse::telemetry::MetricId::TempC);
    appendMetricHistory(eclipse::telemetry::MetricId::HumidityRH);
    appendMetricHistory(eclipse::telemetry::MetricId::PressureHpa);
    appendMetricHistory(eclipse::telemetry::MetricId::CO2ppm);
    appendMetricHistory(eclipse::telemetry::MetricId::RadiationCpm);

    const auto selectedMetric = dashboardVm_.GetSelectedTrendMetric();
    const auto filteredHistory = BuildFilteredHistory(HistoryForMetric(selectedMetric));

    dashboardVm_.Update(
        snapshot_,
        logic_,
        filteredHistory,
        std::chrono::duration_cast<std::chrono::milliseconds>(now - logic_.missionStart_)
    );

    RefreshUi();

    qint64 e2eElapsed = e2eTimer.elapsed();
    qDebug() << "End-to-End Latency (ms):" << e2eElapsed;

    MaybeShowAlertDialog();

}

void MainWindow::SetupTrendWindowButtons()
{
    using TW = eclipse::telemetry::history::TrendWindow;

    connect(ui->graph1mPushButton, &QPushButton::clicked, this, [this]()
        {
            selectedTrendWindow_ = TW::OneMinute;
            RefreshUi();
        });

    connect(ui->graph5mPushButton, &QPushButton::clicked, this, [this]()
        {
            selectedTrendWindow_ = TW::FiveMinutes;
            RefreshUi();
        });

    connect(ui->graph15mPushButton, &QPushButton::clicked, this, [this]()
        {
            selectedTrendWindow_ = TW::FifteenMinutes;
            RefreshUi();
        });

    connect(ui->graph1hPushButton, &QPushButton::clicked, this, [this]()
        {
            selectedTrendWindow_ = TW::OneHour;
            RefreshUi();
        });

    connect(ui->graphAllPushButton, &QPushButton::clicked, this, [this]()
        {
            selectedTrendWindow_ = TW::AllTime;
            RefreshUi();
        });

    //for making checkable buttons mutually exclusive
    trendWindowGroup_ = new QButtonGroup(this);

    trendWindowGroup_->setExclusive(true);

    trendWindowGroup_->addButton(ui->graph1mPushButton);
    trendWindowGroup_->addButton(ui->graph5mPushButton);
    trendWindowGroup_->addButton(ui->graph15mPushButton);
    trendWindowGroup_->addButton(ui->graph1hPushButton);
    trendWindowGroup_->addButton(ui->graphAllPushButton);

    ui->graph5mPushButton->setChecked(true);
}

eclipse::viewmodel::DashboardVm::TrendHistory MainWindow::BuildFilteredHistory(
    const eclipse::viewmodel::DashboardVm::TrendHistory& source
) const
{
    eclipse::viewmodel::DashboardVm::TrendHistory filtered;

    if (source.empty())
    {
        return filtered;
    }

    const auto latest = source.latest();
    if (!latest.has_value())
    {
        return filtered;
    }
    auto duration = eclipse::telemetry::history::WindowDuration(selectedTrendWindow_);
    const auto cutoff = latest->t - duration;

    for (std::size_t i = 0; i < source.size(); ++i)
    {
        const auto& p = source.point(i);

        if (p.t >= cutoff)
        {
            filtered.add(p.t, p.v);
        }
    }

    return filtered;
}

void MainWindow::RefreshUi()
{
    tempCardWidget_.Update(dashboardVm_.GetTempCard());
    humidityCardWidget_.Update(dashboardVm_.GetHumidityCard());
    pressureCardWidget_.Update(dashboardVm_.GetPressureCard());
    co2CardWidget_.Update(dashboardVm_.GetCo2Card());
    radiationCardWidget_.Update(dashboardVm_.GetRadiationCard());

    alertsTableWidget_.Update(dashboardVm_.GetAlerts());

    const auto& trendVm = dashboardVm_.GetTrendPlot();

    QElapsedTimer timer;
    timer.start();

    trendPlot_->SetMetricData(
        QString::fromStdString(trendVm.GetLabel()),
        "Value",
        ConvertTrendHistory(trendVm.GetHistory())
    );

    qint64 elapsed = timer.elapsed();
    qDebug() << "Trend Chart Render Time (ms):" << elapsed;

    setStatusDots();
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
//Added alert dialog helper functoions
std::optional<eclipse::logic::alerts::Alert> MainWindow::FindPopupCandidate() const
{
    const auto& alerts = dashboardVm_.GetAlerts();

    // Two passes: critical first, then warning
    for (auto targetSeverity : {
        eclipse::logic::alerts::AlertSeverity::Critical,
        eclipse::logic::alerts::AlertSeverity::Warning
        })
    {
        for (const auto& alert : alerts)
        {
            if (alert.state != eclipse::logic::alerts::AlertState::Active)
                continue;

            if (alert.severity != targetSeverity)
                continue;

            // Skip alerts whose dialog has already been shown
            if (shownPopupKeys_.count(MakePopupKey(alert)))
                continue;

            return alert;
        }
    }

    return std::nullopt;
}

MainWindow::AlertPopupKey MainWindow::MakePopupKey(
    const eclipse::logic::alerts::Alert& alert
) const
{
    AlertPopupKey key;
    key.type = alert.type;
    key.severity = alert.severity;
    key.metric = alert.metric;
    return key;
}

QString MainWindow::FormatAlertMetric(const eclipse::logic::alerts::Alert& alert) const
{
    if (!alert.metric.has_value())
    {
        return "System";
    }

    const auto& spec = eclipse::telemetry::GetMetricSpec(*alert.metric);
    return QString::fromUtf8(
        spec.displayName.data(),
        static_cast<int>(spec.displayName.size())
    );
}

QString MainWindow::FormatAlertValueNumber(const eclipse::logic::alerts::Alert& alert) const
{
    if (!alert.metric.has_value())
    {
        return "--";
    }

    const auto value = snapshot_.Value(*alert.metric);
    if (!value.has_value())
    {
        return "--";
    }

    return QString::number(*value, 'f', 0);
}

QString MainWindow::FormatAlertValueUnit(
    const eclipse::logic::alerts::Alert& alert
) const
{
    if (!alert.metric.has_value())
    {
        return "";
    }

    const auto& spec =
        eclipse::telemetry::GetMetricSpec(*alert.metric);

    auto symbol = core::units::Symbol(spec.unit);

    return QString::fromUtf8(
        symbol.data(),
        static_cast<int>(symbol.size())
    );
}

QString MainWindow::FormatAlertTimestamp(const eclipse::logic::alerts::Alert& alert) const
{
    Q_UNUSED(alert);
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void MainWindow::FocusAlertsTable()
{
    if (ui->alertTable == nullptr)
    {
        return;
    }

    ui->alertTable->setFocus();

    if (ui->alertTable->rowCount() > 0)
    {
        ui->alertTable->selectRow(0);
        ui->alertTable->scrollToTop();
    }
}

void MainWindow::ShowAlertDialog(const eclipse::logic::alerts::Alert& alert)
{
    //find the index of this alert so AcknowledgePopupAlert can act on it
    const auto& alerts = dashboardVm_.GetAlerts();
    for (std::size_t i = 0; i < alerts.size(); ++i)
    {
        if (MakePopupKey(alerts[i]) == MakePopupKey(alert))
        {
            activePopupAlertIndex_ = i;
            break;
        }
    }

    activePopupKey_ = MakePopupKey(alert);

    telemetryTimer_->stop();

    AlertDialog dialog(this);

    connect(&dialog, &AlertDialog::AcknowledgeRequested,
        this, &MainWindow::AcknowledgePopupAlert);
    connect(&dialog, &AlertDialog::ViewAlertsRequested,
        this, &MainWindow::FocusAlertsTable);

    const QString metric = FormatAlertMetric(alert);
    const QString message = QString::fromStdString(alert.message.empty()
        ? eclipse::logic::alerts::AlertFormatter::Format(alert)
        : alert.message);
    const QString valueNumber = FormatAlertValueNumber(alert);
    const QString valueUnit = FormatAlertValueUnit(alert);
    const QString severity = QString::fromUtf8(
        eclipse::logic::alerts::AlertFormatter::SeverityToString(alert.severity)
    );
    const QString timestamp = FormatAlertTimestamp(alert);

    dialog.SetAlertData(metric, message, valueNumber, valueUnit, severity, timestamp);

    dialog.exec();

    alertDialogOpen_ = false;
    activePopupKey_.reset();

    telemetryTimer_->start(config_.samplePeriodMs);
}

void MainWindow::MaybeShowAlertDialog()
{
    if (alertDialogOpen_)
        return;

    const auto& alerts = dashboardVm_.GetAlerts();
    const auto now = std::chrono::steady_clock::now();
    constexpr auto kDebounce = std::chrono::seconds(15);

    //mark keys that are no longer active with a resolved timestamp
    for (const auto& key : shownPopupKeys_)
    {
        bool stillActive = false;
        for (const auto& alert : alerts)
        {
            if (alert.state == eclipse::logic::alerts::AlertState::Active
                && MakePopupKey(alert) == key)
            {
                stillActive = true;
                break;
            }
        }

        if (!stillActive && popupKeyResolvedAt_.find(key) == popupKeyResolvedAt_.end())
            popupKeyResolvedAt_[key] = now;

        if (stillActive)
            popupKeyResolvedAt_.erase(key); //came back, cancel the resolved timer
    }

    //only erase shown keys that have been gone longer than the debounce window
    for (auto it = shownPopupKeys_.begin(); it != shownPopupKeys_.end(); )
    {
        auto resolved = popupKeyResolvedAt_.find(*it);
        if (resolved != popupKeyResolvedAt_.end()
            && (now - resolved->second) > kDebounce)
        {
            popupKeyResolvedAt_.erase(resolved);
            it = shownPopupKeys_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    const auto candidate = FindPopupCandidate();
    if (!candidate.has_value())
        return;

    alertDialogOpen_ = true;
    shownPopupKeys_.insert(MakePopupKey(*candidate));

    QMetaObject::invokeMethod(this, [this, candidate]() {
        ShowAlertDialog(*candidate);
        }, Qt::QueuedConnection);
}

void MainWindow::AcknowledgePopupAlert()
{
    if (!activePopupAlertIndex_.has_value())
    {
        return;
    }

    logic_.AcknowledgeAlert(*activePopupAlertIndex_);

    //updated with new history logic.
    const auto selectedMetric = dashboardVm_.GetSelectedTrendMetric();
    const auto missionElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - logic_.missionStart_);
    dashboardVm_.Update(
        snapshot_,
        logic_,
        HistoryForMetric(selectedMetric),
        missionElapsed
    );

    RefreshUi();

    activePopupAlertIndex_.reset();
    lastShownPopupAlertIndex_.reset();
}

