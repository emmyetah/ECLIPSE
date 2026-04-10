#include <QDebug>

#include <vector>
#include <numeric>
#include <algorithm>
#include <chrono>

#include "../logic/thresholds/ThresholdEvaluator.h"
#include "../logic/thresholds/ThresholdLevel.h"
#include "../logic/thresholds/ThresholdRule.h"

using namespace eclipse::logic::thresholds;

void RunAlertTimingTests() {

    struct TimingTest {
        const char* name;
        double value;
        ThresholdRule rule;
    };

    //Define rules 
    ThresholdRule tempRule;
    tempRule.normal = { 18.5, 27.5 };
    tempRule.caution = { 18.0, 18.5 };
    tempRule.warning = { 0.0, 18.0 };
    tempRule.critical = { 27.5, 100.0 };

    ThresholdRule humidityRule;
    humidityRule.normal = { 32.0, 68.0 };
    humidityRule.caution = { 30.0, 32.0 };
    humidityRule.warning = { 0.0, 30.0 };
    humidityRule.critical = { 68.0, 100.0 };

    ThresholdRule pressureRule;
    pressureRule.normal = { 950.0, 1050.0 };
    pressureRule.caution = { 900.0, 950.0 };
    pressureRule.warning = { 0.0, 900.0 };
    pressureRule.critical = { 1050.0, 2000.0 };

    ThresholdRule co2Rule;
    co2Rule.normal = { 0.0, 1000.0 };
    co2Rule.caution = { 1000.0, 2000.0 };
    co2Rule.warning = { 2000.0, 3000.0 };
    co2Rule.critical = { 3000.0, 5000.0 };

    ThresholdRule radiationRule;
    radiationRule.normal = { 0.0, 40.0 };
    radiationRule.caution = { 40.0, 80.0 };
    radiationRule.warning = { 80.0, 120.0 };
    radiationRule.critical = { 120.0, 200.0 };

    std::vector<TimingTest> tests = {
        {"Temperature", 28.0, tempRule},
        {"Humidity", 29.0, humidityRule},
        {"Pressure", 1100.0, pressureRule},
        {"CO2", 2500.0, co2Rule},
        {"Radiation", 140.0, radiationRule}
    };

    qDebug() << "========== Alert Timing Tests ==========";

    for (const auto& test : tests) {

        std::vector<double> timings;

        for (int i = 0; i < 20; i++) {

            auto start = std::chrono::high_resolution_clock::now();

            // Repeat operation to improve timing accuracy
            for (int j = 0; j < 100; j++) {
                auto level = ThresholdEvaluator::Evaluate(test.value, test.rule);

                if (level != ThresholdLevel::Warning &&
                    level != ThresholdLevel::Critical) {
                    continue;
                }
            }

            auto end = std::chrono::high_resolution_clock::now();

            auto duration =
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

            timings.push_back(duration / 100); // average per operation
        }

        long long minVal = *std::min_element(timings.begin(), timings.end());
        long long maxVal = *std::max_element(timings.begin(), timings.end());

        double avgVal =
            std::accumulate(timings.begin(), timings.end(), 0.0) / timings.size();

        qDebug() << test.name
            << "| Min:" << minVal << "us"
            << "| Max:" << maxVal << "us"
            << "| Avg:" << avgVal << "us";
    }

    qDebug() << "========================================";
}