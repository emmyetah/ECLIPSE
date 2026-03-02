#pragma once

#include <string>
#include <optional>
#include <vector>
#include <cstdint>

#include "../telemetry/TelemetrySnapshot.h"

namespace eclipse::io::parsing
{
    /**
     *Expected CSV format:
     * -ts_ms,
     * -bme_temp_c,
     * -bme_rh_pct, (rh - relative humidity)
     *- bme_pressure_hpa,
     * -scd_co2_ppm,
     * -scd_temp_c,
     * -scd_rh_pct,
     * -rad_cpm
     *
     * Not planning on using gas data from my bme680
     **/

    
    class TelemetryParser
    {
    public:
        //for parsing a line of data into csv format, NA is shown if data is malformed
        TelemetryParser() = default; // default constructor sice parser has no member variables

        // return type is optional so that if i return std::nullopt if data is malformed and my router can ignore bad lines.
        std::optional<telemetry::TelemetrySnapshot> parseLine(const std::string& line) const;

    private:
        
        //split the line by commas and return individual values in string format
        static std::vector<std::string> splitCsv(const std::string& line);

        //tries to convert numbers to doubles, returns nullopt is token is NA.
        static std::optional<double> toDoubleOrNA(const std::string& token);

        //same as toDoubleOrNa
        static std::optional<int64_t> toInt64OrNA(const std::string& token);

        //remove white space around tokens to prevent bugs.
        static std::string trim(const std::string& s);
        
        //to define what missin means ("NA, "", "nan" etc)
        static bool isNA(const std::string& token);
    };

} 