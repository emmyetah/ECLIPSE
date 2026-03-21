#pragma once

#include <QTableWidget>
#include <vector>
#include <unordered_set> // to prevent embedded for loops
#include <string>
#include <optional>
#include <chrono>

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
        void ClearTable(); // for added clear alerts button

    private:
        //Wraps each alert with an optional resolved mission time, freezes the duration column at a point
        struct CachedAlert
        {
            logic::alerts::Alert alert;
            std::optional<std::chrono::steady_clock::time_point> resolvedAt;
            std::chrono::steady_clock::time_point cachedAt = std::chrono::steady_clock::now();
        };

        QTableWidget* table_ = nullptr;
        //for storing alerts until they are cleared
        std::vector<CachedAlert> cachedAlerts_;
        std::unordered_set<std::string> cachedKeys_; //alongside cachedAlerts_
        

        void SetupTable();
        void RebuildTable();

        static std::string MakeAlertKey(const logic::alerts::Alert& alert);
        

        //format alerts for the table 
        QString FormatTime(const logic::alerts::Alert& alert) const;
        QString FormatMetric(const logic::alerts::Alert& alert) const;
        QString FormatSeverity(const logic::alerts::Alert& alert) const;
        QString FormatDuration(const CachedAlert& entry) const;

        QTableWidgetItem* MakeReadOnlyItem(const QString& text) const;
        void ApplySeverityStyle(QTableWidgetItem* item, logic::alerts::AlertSeverity severity) const;
    };
}