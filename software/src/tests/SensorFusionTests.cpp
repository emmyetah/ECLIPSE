#include <QDebug>
#include <optional>
#include <vector>
#include <cmath>
#include <chrono>

#include "../logic/fusion/SensorFusion.h"
#include "../telemetry/TelemetrySnapshot.h"
#include "../telemetry/TelemetrySample.h"
#include "../telemetry/MetricId.h"

using eclipse::logic::fusion::FusionResult;
using eclipse::logic::fusion::SensorFusion;
using eclipse::telemetry::MetricId;
using eclipse::telemetry::TelemetrySample;
using eclipse::telemetry::TelemetrySnapshot;

struct FusionTestCase {
    const char* testName;

    MetricId metricA;
    MetricId metricB;

    std::optional<double> valueA;
    std::optional<double> valueB;

    double tolerance;

    std::optional<double> expectedValue;
    bool expectedFallback;
    bool expectedMismatch;
};

static bool NearlyEqual(double a, double b, double epsilon = 0.01) {
    return std::fabs(a - b) < epsilon;
}

static void ApplyValue(
    TelemetrySnapshot& snapshot,
    MetricId metric,
    const std::optional<double>& value
) {
    if (!value.has_value()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    TelemetrySample sample(metric, *value, now, true);
    snapshot.Apply(sample);
}

void RunSensorFusionTests() {
    std::vector<FusionTestCase> tests = {
        // Temperature tests
        {
            "Temperature Agreement",
            MetricId::TempC_BME680, MetricId::TempC_SCD30,
            24.0, 25.0,
            3.0,
            24.5, false, false
        },
        {
            "Temperature Mismatch",
            MetricId::TempC_BME680, MetricId::TempC_SCD30,
            24.0, 29.0,
            3.0,
            29.0, false, true
        },
        {
            "Temperature Fallback BME Only",
            MetricId::TempC_BME680, MetricId::TempC_SCD30,
            24.5, std::nullopt,
            3.0,
            24.5, true, false
        },
        {
            "Temperature Fallback SCD Only",
            MetricId::TempC_BME680, MetricId::TempC_SCD30,
            std::nullopt, 25.2,
            3.0,
            25.2, true, false
        },
        {
            "Temperature No Data",
            MetricId::TempC_BME680, MetricId::TempC_SCD30,
            std::nullopt, std::nullopt,
            3.0,
            std::nullopt, false, false
        },

        // Humidity tests
        {
            "Humidity Agreement",
            MetricId::HumidityRH_BME680, MetricId::HumidityRH_SCD30,
            45.0, 47.0,
            5.0,
            46.0, false, false
        },
        {
            "Humidity Mismatch",
            MetricId::HumidityRH_BME680, MetricId::HumidityRH_SCD30,
            45.0, 55.5,
            5.0,
            55.5, false, true
        },
        {
            "Humidity Fallback BME Only",
            MetricId::HumidityRH_BME680, MetricId::HumidityRH_SCD30,
            44.0, std::nullopt,
            5.0,
            44.0, true, false
        },
        {
            "Humidity Fallback SCD Only",
            MetricId::HumidityRH_BME680, MetricId::HumidityRH_SCD30,
            std::nullopt, 48.5,
            5.0,
            48.5, true, false
        },
        {
            "Humidity No Data",
            MetricId::HumidityRH_BME680, MetricId::HumidityRH_SCD30,
            std::nullopt, std::nullopt,
            5.0,
            std::nullopt, false, false
        }
    };

    qDebug() << "========== Sensor Fusion Validation Tests ==========";

    int passed = 0;
    int total = static_cast<int>(tests.size());

    for (const auto& test : tests) {
        TelemetrySnapshot snapshot{};

        ApplyValue(snapshot, test.metricA, test.valueA);
        ApplyValue(snapshot, test.metricB, test.valueB);

        FusionResult result{};

        const bool isTemperatureTest =
            test.metricA == MetricId::TempC_BME680 &&
            test.metricB == MetricId::TempC_SCD30;

        const bool isHumidityTest =
            test.metricA == MetricId::HumidityRH_BME680 &&
            test.metricB == MetricId::HumidityRH_SCD30;

        if (isTemperatureTest) {
            result = SensorFusion::FuseTemperature(snapshot, test.tolerance);
        }
        else if (isHumidityTest) {
            result = SensorFusion::FuseHumidity(snapshot, test.tolerance);
        }
        else {
            qDebug() << test.testName << "| ERROR: Unsupported metric pair";
            continue;
        }

        bool valuePass = false;

        if (!test.expectedValue.has_value() && !result.value.has_value()) {
            valuePass = true;
        }
        else if (test.expectedValue.has_value() && result.value.has_value()) {
            valuePass = NearlyEqual(*test.expectedValue, *result.value);
        }

        const bool fallbackPass = (result.usedFallback == test.expectedFallback);
        const bool mismatchPass = (result.mismatch == test.expectedMismatch);
        const bool overallPass = valuePass && fallbackPass && mismatchPass;

        if (overallPass) {
            ++passed;
        }

        QString actualValueText = result.value.has_value()
            ? QString::number(*result.value, 'f', 2)
            : "None";

        QString expectedValueText = test.expectedValue.has_value()
            ? QString::number(*test.expectedValue, 'f', 2)
            : "None";

        qDebug().noquote()
            << QString("%1 | Expected Value: %2 | Actual Value: %3 | Expected Fallback: %4 | Actual Fallback: %5 | Expected Mismatch: %6 | Actual Mismatch: %7 | Result: %8")
            .arg(test.testName)
            .arg(expectedValueText)
            .arg(actualValueText)
            .arg(test.expectedFallback ? "true" : "false")
            .arg(result.usedFallback ? "true" : "false")
            .arg(test.expectedMismatch ? "true" : "false")
            .arg(result.mismatch ? "true" : "false")
            .arg(overallPass ? "Pass" : "Fail");
    }

    qDebug() << "===================================================";
    qDebug() << "Passed" << passed << "out of" << total << "tests";
}