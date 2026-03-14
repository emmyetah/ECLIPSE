#include "TelemetryParser.h"

#include <sstream>
#include <algorithm>
#include <cctype>

#include "../../telemetry/MetricId.h"
#include "../../core/types/Time.h"

namespace eclipse::io::parsing
{
    std::optional<std::vector<telemetry::TelemetrySample>> TelemetryParser::parseLine(const std::string& line) const
    {
        //split csv into tokens
        std::vector<std::string> tokens = splitCsv(line);

        //expected 8 values:
        //ts_ms,bme_temp_c,bme_rh_pct,bme_pressure_hpa,scd_co2_ppm,scd_temp_c,scd_rh_pct,rad_cpm
        if (tokens.size() != 8) {
            return std::nullopt;
        }

        //timestamp must be present and valid
        std::optional<int64_t> tsMs = toInt64OrNA(tokens[0]);
        if (!tsMs.has_value()) {
            return std::nullopt;
        }

        std::vector<telemetry::TelemetrySample> samples;

        core::time::TimePoint timestamp = core::time::Now();

        //BME680
        if (auto value = toDoubleOrNA(tokens[1]); value.has_value()) {
            samples.emplace_back(
                telemetry::MetricId::TempC_BME680,
                *value,
                timestamp
            );
        }

        if (auto value = toDoubleOrNA(tokens[2]); value.has_value()) {
            samples.emplace_back(
                telemetry::MetricId::HumidityRH_BME680,
                *value,
                timestamp
            );
        }

        if (auto value = toDoubleOrNA(tokens[3]); value.has_value()) {
            samples.emplace_back(
                telemetry::MetricId::PressureHpa,
                *value,
                timestamp
            );
        }

        //SCD30
        if (auto value = toDoubleOrNA(tokens[4]); value.has_value()) {
            samples.emplace_back(
                telemetry::MetricId::CO2ppm,
                *value,
                timestamp
            );
        }

        if (auto value = toDoubleOrNA(tokens[5]); value.has_value()) {
            samples.emplace_back(
                telemetry::MetricId::TempC_SCD30,
                *value,
                timestamp
            );
        }

        if (auto value = toDoubleOrNA(tokens[6]); value.has_value()) {
            samples.emplace_back(
                telemetry::MetricId::HumidityRH_SCD30,
                *value,
                timestamp
            );
        }

        //Radiation
        if (auto value = toInt64OrNA(tokens[7]); value.has_value()) {
            samples.emplace_back(
                telemetry::MetricId::RadiationCpm,
                static_cast<double>(*value),
                timestamp
            );
        }

        return samples;
    }

    std::vector<std::string> TelemetryParser::splitCsv(const std::string& line)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, ',')) {
            tokens.push_back(trim(token));
        }

        return tokens;
    }

    std::optional<double> TelemetryParser::toDoubleOrNA(const std::string& token)
    {
        std::string cleaned = trim(token);

        if (isNA(cleaned)) {
            return std::nullopt;
        }

        try {
            return std::stod(cleaned);
        }
        catch (...) {
            return std::nullopt;
        }
    }

    std::optional<int64_t> TelemetryParser::toInt64OrNA(const std::string& token)
    {
        std::string cleaned = trim(token);

        if (isNA(cleaned)) {
            return std::nullopt;
        }

        try {
            return std::stoll(cleaned);
        }
        catch (...) {
            return std::nullopt;
        }
    }

    std::string TelemetryParser::trim(const std::string& s)
    {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            start++;
        }

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            end--;
        }

        return s.substr(start, end - start);
    }

    bool TelemetryParser::isNA(const std::string& token)
    {
        std::string cleaned = trim(token);
        std::string lower = cleaned;

        std::transform(
            lower.begin(),
            lower.end(),
            lower.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        );

        return lower.empty()
            || lower == "na"
            || lower == "n/a"
            || lower == "nan"
            || lower == "null";
    }
}