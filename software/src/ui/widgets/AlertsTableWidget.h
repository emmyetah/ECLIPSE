#pragma once

#include <QTableWidget>
#include <vector>

#include "../../logic/alerts/Alert.h"

namespace eclipse::ui::widgets
{
    class AlertsTableWidget
    {
    public:
        AlertsTableWidget() = default;

        void Bind(QTableWidget* table);
        //adds alerts to the alerts table
        void Update(const std::vector<logic::alerts::Alert>& alerts);

    private:
        QTableWidget* table_ = nullptr;

        void SetupTable();

        //format alerts for the table 
        QString FormatTime(const logic::alerts::Alert& alert) const;
        QString FormatMetric(const logic::alerts::Alert& alert) const;
        QString FormatSeverity(const logic::alerts::Alert& alert) const;
        QString FormatDuration(const logic::alerts::Alert& alert) const;

        QTableWidgetItem* MakeReadOnlyItem(const QString& text) const;
        void ApplySeverityStyle(QTableWidgetItem* item, logic::alerts::AlertSeverity severity) const;
    };
}