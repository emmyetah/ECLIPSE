#include "AlertsTableWidget.h"

#include "../../logic/alerts/AlertFormatter.h"
#include "../../telemetry/MetricSpec.h"

#include "../../core/utils/Format.h"

#include <QHeaderView>
#include <QBrush>
#include <QColor>

#include <chrono>

namespace eclipse::ui::widgets
{
    void AlertsTableWidget::Bind(QTableWidget* table)
    {
        table_ = table;
        SetupTable();
    }

    void AlertsTableWidget::SetupTable()
    {
        //if table name doesn'ty link return
        if (table_ == nullptr) return;

        //setting up 4 columns 
        table_->setColumnCount(4);

        QStringList headers;
        headers << "Time" << "Metric" << "Severity" << "Duration";
        table_->setHorizontalHeaderLabels(headers);

        //setting table properties
        table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table_->setSelectionBehavior(QAbstractItemView::SelectRows);
        table_->setSelectionMode(QAbstractItemView::SingleSelection);
        table_->setAlternatingRowColors(false);
        table_->verticalHeader()->setVisible(false);
        table_->setShowGrid(false);

        table_->horizontalHeader()->setStretchLastSection(true);
        table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    void AlertsTableWidget::Update(const std::vector<logic::alerts::Alert>& alerts)
    {
        if (table_ == nullptr) return;

        table_->setRowCount(0);

        int row = 0;

        //for loop to add the alerts to the table.
        for (const auto& alert : alerts)
        {
            if (alert.state == logic::alerts::AlertState::Cleared)
            {
                continue;
            }

            table_->insertRow(row);

            QTableWidgetItem* timeItem = MakeReadOnlyItem(FormatTime(alert));
            QTableWidgetItem* metricItem = MakeReadOnlyItem(FormatMetric(alert));
            QTableWidgetItem* severityItem = MakeReadOnlyItem(FormatSeverity(alert));
            QTableWidgetItem* durationItem = MakeReadOnlyItem(FormatDuration(alert));

            ApplySeverityStyle(severityItem, alert.severity);

            table_->setItem(row, 0, timeItem);
            table_->setItem(row, 1, metricItem);
            table_->setItem(row, 2, severityItem);
            table_->setItem(row, 3, durationItem);

            row++;
        }
    }

    QString AlertsTableWidget::FormatTime(const logic::alerts::Alert& alert) const
    {
        const auto text = core::format::MissionTime(alert.missionElapsedAtStart);
        return QString::fromStdString(text);
    }

    QString AlertsTableWidget::FormatMetric(const logic::alerts::Alert& alert) const
    {
        if (alert.metric.has_value())
        {
            const auto& spec = eclipse::telemetry::GetMetricSpec(*alert.metric);

            return QString::fromUtf8(
                spec.displayName.data(),
                static_cast<int>(spec.displayName.size())
            );
        }

        switch (alert.type)
        {
        case logic::alerts::AlertType::InvalidReading:
        case logic::alerts::AlertType::SensorOffline:
        case logic::alerts::AlertType::SensorStale:
        case logic::alerts::AlertType::SensorMismatch:
            return "Sensor";

        default:
            break;
        }

        return QString::fromUtf8(logic::alerts::AlertFormatter::TypeToString(alert.type));
    }
   

    QString AlertsTableWidget::FormatSeverity(const logic::alerts::Alert& alert) const
    {
        return QString::fromUtf8(
            logic::alerts::AlertFormatter::SeverityToString(alert.severity)
        );
    }

    QString AlertsTableWidget::FormatDuration(const logic::alerts::Alert& alert) const
    {
        const auto now = std::chrono::steady_clock::now();
        const auto seconds =
            std::chrono::duration_cast<std::chrono::seconds>(now - alert.timestamp).count();

        const long long hours = seconds / 3600;
        const long long minutes = (seconds % 3600) / 60;
        const long long secs = seconds % 60;

        if (hours > 0)
        {
            return QString("%1h %2m").arg(hours).arg(minutes);
        }

        if (minutes > 0)
        {
            return QString("%1m %2s").arg(minutes).arg(secs);
        }

        return QString("%1s").arg(secs);
    }

    QTableWidgetItem* AlertsTableWidget::MakeReadOnlyItem(const QString& text) const
    {
        QTableWidgetItem* item = new QTableWidgetItem(text);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        return item;
    }

    void AlertsTableWidget::ApplySeverityStyle(
        QTableWidgetItem* item,
        logic::alerts::AlertSeverity severity
    ) const
    {
        if (item == nullptr) return;

        switch (severity)
        {
        case logic::alerts::AlertSeverity::Information:
            item->setForeground(QBrush(QColor("#57C785")));
            break;

        case logic::alerts::AlertSeverity::Warning:
            item->setForeground(QBrush(QColor("#FF9F43")));
            break;

        case logic::alerts::AlertSeverity::Critical:
            item->setForeground(QBrush(QColor("#FF5C5C")));
            break;

        default:
            break;
        }
    }
}