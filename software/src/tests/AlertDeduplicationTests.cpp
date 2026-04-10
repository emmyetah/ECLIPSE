#include <QDebug>
#include <vector>

#include "../logic/alerts/AlertDuplicator.h"
#include "../logic/alerts/Alert.h"
#include "../logic/alerts/AlertSeverity.h"
#include "../logic/alerts/AlertType.h"
#include "../logic/alerts/AlertState.h"
#include "../telemetry/MetricId.h"

using namespace eclipse::logic::alerts;
using eclipse::telemetry::MetricId;

Alert CreateAlert(
    AlertType type,
    AlertSeverity severity,
    MetricId metric,
    AlertState state = AlertState::Active
) {
    Alert a;
    a.type = type;
    a.severity = severity;
    a.metric = metric;
    a.state = state;
    return a;
}

void RunAlertDeduplicationTests() {

    qDebug() << "========== Alert Deduplication Tests ==========";

    int passed = 0;
    int total = 4;

    // Existing active alert
    std::vector<Alert> existing = {
        CreateAlert(AlertType::ThresholdBreach, AlertSeverity::Warning, MetricId::TempC)
    };

    // 1. Exact duplicate
    {
        Alert candidate = CreateAlert(AlertType::ThresholdBreach, AlertSeverity::Warning, MetricId::TempC);

        bool isDuplicate = AlertDuplicator::IsDuplicate(candidate, existing);
        bool pass = (isDuplicate == true);

        if (pass) ++passed;

        qDebug() << "Exact Duplicate | Result:" << (pass ? "Pass" : "Fail");
    }

    // 2. Different severity
    {
        Alert candidate = CreateAlert(AlertType::ThresholdBreach, AlertSeverity::Critical, MetricId::TempC);

        bool isDuplicate = AlertDuplicator::IsDuplicate(candidate, existing);
        bool pass = (isDuplicate == false);

        if (pass) ++passed;

        qDebug() << "Different Severity | Result:" << (pass ? "Pass" : "Fail");
    }

    // 3. Different metric
    {
        Alert candidate = CreateAlert(AlertType::ThresholdBreach, AlertSeverity::Warning, MetricId::CO2ppm);

        bool isDuplicate = AlertDuplicator::IsDuplicate(candidate, existing);
        bool pass = (isDuplicate == false);

        if (pass) ++passed;

        qDebug() << "Different Metric | Result:" << (pass ? "Pass" : "Fail");
    }

    // 4. Inactive existing alert
    {
        std::vector<Alert> inactiveList = {
            CreateAlert(AlertType::ThresholdBreach, AlertSeverity::Warning, MetricId::TempC, AlertState::Cleared)
        };

        Alert candidate = CreateAlert(AlertType::ThresholdBreach, AlertSeverity::Warning, MetricId::TempC);

        bool isDuplicate = AlertDuplicator::IsDuplicate(candidate, inactiveList);
        bool pass = (isDuplicate == false);

        if (pass) ++passed;

        qDebug() << "Inactive Alert | Result:" << (pass ? "Pass" : "Fail");
    }

    qDebug() << "===============================================";
    qDebug() << "Passed" << passed << "out of" << total << "tests";
}