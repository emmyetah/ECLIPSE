#include <QDebug>
#include <vector>
#include <string>

#include "../logic/thresholds/ThresholdEvaluator.h"
#include "../logic/alerts/Alert.h"
#include "../logic/alerts/AlertSeverity.h"
#include "../logic/alerts/AlertType.h"
#include "../logic/alerts/AlertFormatter.h"
#include "../telemetry/MetricId.h"

using eclipse::logic::thresholds::ThresholdEvaluator;
using eclipse::logic::thresholds::ThresholdLevel;
using eclipse::logic::thresholds::ThresholdRule;

using eclipse::logic::alerts::Alert;
using eclipse::logic::alerts::AlertSeverity;
using eclipse::logic::alerts::AlertType;

using eclipse::telemetry::MetricId;

struct AlertTestCase {
    const char* metricName;
    MetricId metricId;
    double input;

    ThresholdRule rule;

    ThresholdLevel expectedLevel;
    bool expectAlert;
    AlertSeverity expectedSeverity;
};

AlertSeverity MapToSeverity(ThresholdLevel level) {
    if (level == ThresholdLevel::Warning) {
        return AlertSeverity::Warning;
    }
    if (level == ThresholdLevel::Critical) {
        return AlertSeverity::Critical;
    }
    return AlertSeverity::Unknown;
}

void RunAlertAccuracyTests() {

    // Example rule (you can replace with your real ThresholdSet later)
    ThresholdRule tempRule;

    tempRule.normal = { 18.5, 27.5 };
    tempRule.caution = { 18.0, 18.5 };
    tempRule.warning = { 0.0, 18.0 };
    tempRule.critical = { 27.5, 100.0 };

    std::vector<AlertTestCase> tests = {

        {"Temperature", MetricId::TempC_BME680, 22.0, tempRule,
         ThresholdLevel::Normal, false, AlertSeverity::Unknown},

        {"Temperature", MetricId::TempC_BME680, 17.5, tempRule,
         ThresholdLevel::Warning, true, AlertSeverity::Warning},

        {"Temperature", MetricId::TempC_BME680, 28.0, tempRule,
         ThresholdLevel::Critical, true, AlertSeverity::Critical}
    };

    qDebug() << "========== Alert Trigger Accuracy Tests ==========";

    int passed = 0;

    for (const auto& test : tests) {

        ThresholdLevel level =
            ThresholdEvaluator::Evaluate(test.input, test.rule);

        bool alertTriggered =
            (level == ThresholdLevel::Warning ||
                level == ThresholdLevel::Critical);

        AlertSeverity severity = MapToSeverity(level);

        bool pass =
            (level == test.expectedLevel) &&
            (alertTriggered == test.expectAlert) &&
            (severity == test.expectedSeverity);

        if (pass) {
            ++passed;
        }

        qDebug()
            << test.metricName
            << "| Input:" << test.input
            << "| Level:" << static_cast<int>(level)
            << "| Alert:" << alertTriggered
            << "| Severity:" << static_cast<int>(severity)
            << "| Result:" << (pass ? "Pass" : "Fail");
    }

    qDebug() << "===============================================";
    qDebug() << "Passed" << passed << "out of" << tests.size() << "tests";
}