#include <QDebug>
#include <vector>
#include <string>

#include "../config/ThresholdConfig.h"
#include "../logic/thresholds/ThresholdEvaluator.h"
#include "../logic/thresholds/ThresholdLevel.h"
#include "../telemetry/MetricId.h"

using eclipse::EnvironmentProfile;
using eclipse::ThresholdConfig;
using eclipse::BuildThresholdSet;
using eclipse::logic::thresholds::ThresholdEvaluator;
using eclipse::logic::thresholds::ThresholdLevel;
using eclipse::logic::thresholds::ThresholdRule;
using eclipse::telemetry::MetricId;

struct ThresholdTestCase {
    const char* metricName;
    MetricId metricId;
    double inputValue;
    ThresholdLevel expected;
};

static const char* LevelToString(ThresholdLevel level) {
    switch (level) {
    case ThresholdLevel::Normal:   return "Normal";
    case ThresholdLevel::Caution:  return "Caution";
    case ThresholdLevel::Warning:  return "Warning";
    case ThresholdLevel::Critical: return "Critical";
    default:                       return "Unknown";
    }
}

void RunThresholdClassificationTests() {
    ThresholdConfig cfg = ThresholdConfig::defaults(EnvironmentProfile::Earth);
    auto thresholdSet = BuildThresholdSet(cfg, EnvironmentProfile::Earth);

    std::vector<ThresholdTestCase> tests = {
        //Temperature
        {"Temperature", MetricId::TempC, 22.0, ThresholdLevel::Normal},
        {"Temperature", MetricId::TempC, 18.2, ThresholdLevel::Caution},
        {"Temperature", MetricId::TempC, 17.5, ThresholdLevel::Warning},
        {"Temperature", MetricId::TempC, 27.8, ThresholdLevel::Critical},

        //Humidity
        {"Humidity", MetricId::HumidityRH, 50.0, ThresholdLevel::Normal},
        {"Humidity", MetricId::HumidityRH, 31.0, ThresholdLevel::Caution},
        {"Humidity", MetricId::HumidityRH, 29.0, ThresholdLevel::Warning},
        {"Humidity", MetricId::HumidityRH, 69.0, ThresholdLevel::Critical},

        //Pressure
        {"Pressure", MetricId::PressureHpa, 1000.0, ThresholdLevel::Normal},
        {"Pressure", MetricId::PressureHpa, 920.0, ThresholdLevel::Caution},
        {"Pressure", MetricId::PressureHpa, 850.0, ThresholdLevel::Warning},
        {"Pressure", MetricId::PressureHpa, 1075.0, ThresholdLevel::Critical},

        //CO2
        {"CO2", MetricId::CO2ppm, 600.0, ThresholdLevel::Normal},
        {"CO2", MetricId::CO2ppm, 1500.0, ThresholdLevel::Caution},
        {"CO2", MetricId::CO2ppm, 2500.0, ThresholdLevel::Warning},
        {"CO2", MetricId::CO2ppm, 3500.0, ThresholdLevel::Critical},

        //Radiation
        {"Radiation", MetricId::RadiationCpm, 20.0, ThresholdLevel::Normal},
        {"Radiation", MetricId::RadiationCpm, 60.0, ThresholdLevel::Caution},
        {"Radiation", MetricId::RadiationCpm, 100.0, ThresholdLevel::Warning},
        {"Radiation", MetricId::RadiationCpm, 140.0, ThresholdLevel::Critical}
    };

    qDebug() << "========== Threshold Classification Tests ==========";

    int passCount = 0;

    for (const auto& test : tests) {
        const ThresholdRule* rule = thresholdSet.GetRule(test.metricId);

        if (rule == nullptr) {
            qDebug() << test.metricName
                << "| Input:" << test.inputValue
                << "| ERROR: No threshold rule found";
            continue;
        }

        ThresholdLevel actual = ThresholdEvaluator::Evaluate(test.inputValue, *rule);
        bool pass = (actual == test.expected);

        if (pass) {
            ++passCount;
        }

        qDebug() << test.metricName
            << "| Input:" << test.inputValue
            << "| Expected:" << LevelToString(test.expected)
            << "| Actual:" << LevelToString(actual)
            << "| Result:" << (pass ? "Pass" : "Fail");
    }

    qDebug() << "===============================================";
    qDebug() << "Passed" << passCount << "out of" << static_cast<int>(tests.size()) << "tests";
}