//unit enums & format helpers
#pragma once

#include <string_view>


namespace core::units {

    //enum class to hold all units I will need
    enum class Unit {
        none = 0,

        //Temperature
        celsius,
        kelvin,

        //Pressure
        pascal,
        hectopascal,   //hPa (common for barometric pressure)
        kilopascal,    //kPa

        // Humidity
        percent,

        // Gas concentration
        ppm,           // parts per million
        ppb,           // parts per billion

        // Radiation
        microsievert_per_hour, // uSv/h

        // Electrical / power (not needed but added in case of expansion)
        volt,
        ampere,
        watt,

        // Generic / other
        meters,
        seconds
    };
    //helper fucntion to return the symbol for a unit
    std::string_view Symbol(Unit u);

    // Longer label (e.g., "celsius", "hectopascal").
    // string_view is also better as it doesn't allocate memeory & keeps the .length() on file.
    std::string_view Name(Unit u);

} // namespace core::units
