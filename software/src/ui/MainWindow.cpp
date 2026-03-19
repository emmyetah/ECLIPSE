#include "MainWindow.h"
#include "./ui_mainwindow.h"
#include "widgets/TrendPltWidget.h"
#include "../core/types/Time.h"

#include "dialogs/AlertDialog.h"
#include "../logic/alerts/AlertFormatter.h"
#include "../telemetry/MetricSpec.h"

#include <QDateTime>
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
    //default mode = Earth
    logic_.SetMode(eclipse::logic::mode::Mode::Earth);

    //highlight Earth button
    ui->earthModePushButton->setChecked(true);
    ui->spaceCapsulePushButton->setChecked(false);

    //connect Earth button
    connect(ui->earthModePushButton, &QPushButton::clicked, this, [this]()
        {
            logic_.SetMode(eclipse::logic::mode::Mode::Earth);

            //clear metric history when mode swtches
            for (auto& history : metricHistories_)
            {
                history.clear();
            }

            ui->earthModePushButton->setChecked(true);
            ui->spaceCapsulePushButton->setChecked(false);
        });

    //connect Space button
    connect(ui->spaceCapsulePushButton, &QPushButton::clicked, this, [this]()
        {
            logic_.SetMode(eclipse::logic::mode::Mode::Space);

            for (auto& history : metricHistories_)
            {
                history.clear();
            }

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

    for (const auto& sample : *parsed)
    {
        snapshot_.Apply(sample);
    } 

    ApplyFusedMetricsToSnapshot();

    qDebug() << "Temp value exists:" << snapshot_.Value(eclipse::telemetry::MetricId::TempC).has_value();
    qDebug() << "Humidity value exists:" << snapshot_.Value(eclipse::telemetry::MetricId::HumidityRH).has_value();
    qDebug() << "Pressure value exists:" << snapshot_.Value(eclipse::telemetry::MetricId::PressureHpa).has_value();
    qDebug() << "CO2 value exists:" << snapshot_.Value(eclipse::telemetry::MetricId::CO2ppm).has_value();
    qDebug() << "Radiation value exists:" << snapshot_.Value(eclipse::telemetry::MetricId::RadiationCpm).has_value();

    
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
        filteredHistory
    );

    RefreshUi();
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

    trendPlot_->SetMetricData(
        QString::fromStdString(trendVm.GetLabel()),
        "Value",
        ConvertTrendHistory(trendVm.GetHistory())
    );
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

    //active critical first
    for (const auto& alert : alerts)
    {
        if (alert.state == eclipse::logic::alerts::AlertState::Active &&
            alert.severity == eclipse::logic::alerts::AlertSeverity::Critical)
        {
            return alert;
        }
    }

    //then active warning
    for (const auto& alert : alerts)
    {
        if (alert.state == eclipse::logic::alerts::AlertState::Active &&
            alert.severity == eclipse::logic::alerts::AlertSeverity::Warning)
        {
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
    {
        return;
    }

    const auto candidate = FindPopupCandidate();
    if (!candidate.has_value())
    {
        return;
    }

    const auto key = MakePopupKey(*candidate);

    //do not spam the same still-active popup every refresh
    if (lastShownPopupKey_.has_value() && *lastShownPopupKey_ == key)
    {
        return;
    }

    alertDialogOpen_ = true;
    lastShownPopupKey_ = key;

    // Defer ShowAlertDialog out of the timer callback stack
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
    dashboardVm_.Update(
        snapshot_,
        logic_,
        HistoryForMetric(selectedMetric)
    );

    RefreshUi();

    activePopupAlertIndex_.reset();
    lastShownPopupAlertIndex_.reset();
}