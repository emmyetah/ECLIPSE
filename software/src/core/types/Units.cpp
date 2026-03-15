#include "Units.h"


namespace core::units {

    std::string_view Symbol(Unit u)
    {
        switch (u)
        {
        case Unit::none: return "";

        case Unit::celsius:return "\u00B0C";
        case Unit::kelvin: return "K";

        case Unit::pascal: return "Pa";
        case Unit::hectopascal: return "hPa";
        case Unit::kilopascal: return "kPa";

        case Unit::percent: return "%";

        case Unit::ppm: return "ppm";
        case Unit::ppb: return "ppb";

        case Unit::microsievert_per_hour: return "µSv/h";
        case Unit::cpm: return "CPM";
        case Unit::cps: return "CPS";

        case Unit::volt: return "V";
        case Unit::ampere: return "A";
        case Unit::watt: return "W";

        case Unit::meters: return "m";
        case Unit::seconds: return "s";

        default: return "";
        }
    }

    std::string_view Name(Unit u)
    {
        switch (u)
        {
        case Unit::none: return "none";

        case Unit::celsius: return "celsius";
        case Unit::kelvin: return "kelvin";

        case Unit::pascal: return "pascal";
        case Unit::hectopascal: return "hectopascal";
        case Unit::kilopascal: return "kilopascal";

        case Unit::percent: return "percent";

        case Unit::ppm: return "parts per million";
        case Unit::ppb: return "parts per billion";

        case Unit::microsievert_per_hour: return "microsievert per hour";
        case Unit::cpm: return "counts per minute";
        case Unit::cps: return "counts per second";

        case Unit::volt: return "volt";
        case Unit::ampere: return "ampere";
        case Unit::watt: return "watt";

        case Unit::meters: return "meters";
        case Unit::seconds: return "seconds";

        default: return "unknown";
        }
    }

}