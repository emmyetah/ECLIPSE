//value to string formatting, precision rules per sensor
//co2 no decimals, temp 2 decemals etc.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "../types/Time.h"
#include "../types/Units.h"

namespace core::format {
    
    // Returned when data is missing / invalid / not available
    inline constexpr std::string_view kNA = "N/A";

    // Formatting style options for sensor values.
    struct FormatSpec {
        // If set, overrides default decimals for the value.
        std::optional<int> decimalsOverride = std::nullopt;

        // Append a unit  (e.g. "23.40 °C"). If false, just the number (e.g. "23.40").
        bool includeUnit = true;

        // Add a space before the unit symbol ("23.4 °C") vs ("23.4°C").
        bool spaceBeforeUnit = true;

        // If true, trims trailing zeros for decimals (e.g. 2.50 -> "2.5", 2.00 -> "2").
        bool trimTrailingZeros = false;
    };

    // helpers

    // Format a floating point number with fixed decimals. (round up based on the amount of decimals) optionally trims if trim is true
    // Example: (2.3456, 2) -> "2.35" 
    std::string Fixed(double value, int decimals, bool trim);

    // Formats an integer into a string
    std::string Integer(int64_t value);

    //possibly need to add stoi() or stod() function, but not practical

    // Format a numeric value with an associated unit (symbol appended by default).
    // Example: value=23.4, unit=celsius -> "23.4 °C"
    std::string Value(std::optional<double> value, core::units::Unit unit, const FormatSpec& spec = {});

    // -------- Time formatting --------

    // Converts elapsed milliseconds into "HH:MM:SS" (mission time).
    std::string MissionTime(core::time::Milliseconds elapsed);

    // Formats a MissionTime struct into "HH:MM:SS".
    std::string MissionTime(const core::time::MissionTime& t);

}
