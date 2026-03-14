
#include "Format.h"

#include <iomanip>
#include <sstream>

namespace core::format {

    namespace
    {
        int DefaultDecimals(core::units::Unit unit)
        {
            switch (unit)
            {
            case core::units::Unit::celsius:
            case core::units::Unit::kelvin:
                return 2;

            case core::units::Unit::pascal:
            case core::units::Unit::hectopascal:
            case core::units::Unit::kilopascal:
                return 2;

            case core::units::Unit::percent:
                return 1;

            case core::units::Unit::ppm:
            case core::units::Unit::ppb:
                return 0;

            case core::units::Unit::microsievert_per_hour:
                return 2;

            case core::units::Unit::volt:
            case core::units::Unit::ampere:
            case core::units::Unit::watt:
                return 2;

            case core::units::Unit::meters:
                return 2;

            case core::units::Unit::seconds:
                return 0;

            case core::units::Unit::none:
            default:
                return 2;
            }
        }

        void TrimTrailingZeros(std::string& text)
        {
            std::size_t dotPos = text.find('.');
            if (dotPos == std::string::npos) {
                return;
            }

            while (!text.empty() && text.back() == '0') {
                text.pop_back();
            }

            if (!text.empty() && text.back() == '.') {
                text.pop_back();
            }
        }
    }

    std::string Fixed(double value, int decimals, bool trim)
    {
        if (decimals < 0) {
            decimals = 0;
        }

        std::ostringstream out;
        out << std::fixed << std::setprecision(decimals) << value;

        std::string text = out.str();

        if (trim) {
            TrimTrailingZeros(text);
        }

        return text;
    }

    std::string Integer(int64_t value)
    {
        return std::to_string(value);
    }

    std::string Value(std::optional<double> value, core::units::Unit unit, const FormatSpec& spec)
    {
        if (!value.has_value()) {
            return std::string(kNA);
        }

        int decimals = spec.decimalsOverride.has_value()
            ? *spec.decimalsOverride
            : DefaultDecimals(unit);

        std::string text = Fixed(*value, decimals, spec.trimTrailingZeros);

        if (!spec.includeUnit) {
            return text;
        }

        std::string_view symbol = core::units::Symbol(unit);

        if (symbol.empty()) {
            return text;
        }

        if (spec.spaceBeforeUnit) {
            return text + " " + std::string(symbol);
        }

        return text + std::string(symbol);
    }

    std::string MissionTime(core::time::Milliseconds elapsed)
    {
        return core::time::FormatMissionTime(core::time::ToMissionTime(elapsed));
    }

    std::string MissionTime(const core::time::MissionTime& t)
    {
        return core::time::FormatMissionTime(t);
    }

}