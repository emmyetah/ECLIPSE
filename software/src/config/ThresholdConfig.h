#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace eclipse {

    //enumbers for thresholds. defines how the threshold behaves. not all metrics behave the same CO2 doesn't need a low limit but temp does.
    enum class ThresholdMode : uint8_t {
        HighOnly,   //only upper limits matter
        LowOnly,  //only lower limits matter  (none of my sensors require this but keeping for principle - no constructor for this as not needed)
        Range    //both sides matter   
    };

    //enum for health status, I can initalise the values here for colour sorting.
    enum class HealthStatus : uint8_t {
        Nominal = 0, //(normal)
        Warning = 1,
        Critical = 2
    };

    //for seperating thresholds.
    enum class EnvironmentProfile : uint8_t {
        Earth,
        Space
    };

    struct ThresholdBand {
        //threshold values
        std::optional<double> lowWarn;
        std::optional<double> lowCrit;
        std::optional<double> highWarn;
        std::optional<double> highCrit;

        ThresholdMode mode = ThresholdMode::Range;

        // Convenience constructors
        //sets the warning and critcal values for metrics that only get warned when the exceed certain values not drop below them.
        static ThresholdBand HighOnly(double warn, double crit) {
            ThresholdBand t;
            t.mode = ThresholdMode::HighOnly;
            t.highWarn = warn;
            t.highCrit = crit;
            return t;
        }
        //sets warning and critical for low and high values. 
        static ThresholdBand Range(double lowWarn, double lowCrit, double highWarn, double highCrit) {
            ThresholdBand t;
            t.mode = ThresholdMode::Range;
            t.lowWarn = lowWarn;
            t.lowCrit = lowCrit;
            t.highWarn = highWarn;
            t.highCrit = highCrit;
            return t;
        }
    };
    //learned that the inline type allows multiple definitions in cpp files as they are the same definition.
    //inputs threshold definitions & sensor readinf and returns the health status.
    inline HealthStatus evaluateThreshold(const ThresholdBand& t, double value) {

        //helper lambdas for convenience

        //funcitons take a double, checks if it has a value with the has_value() fucniton then compares it the the threshold. 
        auto isLess = [&](const std::optional<double>& x) { return x.has_value() && value < *x; };
        auto isGreater = [&](const std::optional<double>& x) { return x.has_value() && value > *x; };

        //check critical first (more severe wins)
        //if the mode threshold is low only or range, and it's less than critical return a critical health status
        if (t.mode == ThresholdMode::LowOnly || t.mode == ThresholdMode::Range) {
            if (isLess(t.lowCrit)) return HealthStatus::Critical;
        }
        if (t.mode == ThresholdMode::HighOnly || t.mode == ThresholdMode::Range) {
            if (isGreater(t.highCrit)) return HealthStatus::Critical;
        }

        //thencheck warning
        if (t.mode == ThresholdMode::LowOnly || t.mode == ThresholdMode::Range) {
            if (isLess(t.lowWarn)) return HealthStatus::Warning;
        }
        if (t.mode == ThresholdMode::HighOnly || t.mode == ThresholdMode::Range) {
            if (isGreater(t.highWarn)) return HealthStatus::Warning;
        }

        //otherwise return a normal(nomial) health stats if not warning or critical
        return HealthStatus::Nominal;
    }
    //groups all metric thresholds together.
    struct ThresholdConfig {
        //my 5 metrics 
        ThresholdBand temperatureC;
        ThresholdBand humidityRH;
        ThresholdBand pressureHpa;
        ThresholdBand co2Ppm;
        ThresholdBand radiationCpm; //haven't got rad detector yet, might do cps instead

        //setting deafult values for eath and space. 
        static ThresholdConfig defaults(EnvironmentProfile profile) {
            ThresholdConfig cfg;
            switch (profile) {
                case EnvironmentProfile::Earth:{
                    //Standard indoor habitable building thresholds based on csv open source dataset for iot
                    cfg.temperatureC = ThresholdBand::Range(18.5, 18.0, 27.5.0, 28.0);
                    cfg.humidityRH = ThresholdBand::Range(32.0, 30.0, 68.0, 70.0);
                    cfg.pressureHpa = ThresholdBand::Range(950.0, 900.0, 1050.0, 1100.0);
                    cfg.co2Ppm = ThresholdBand::HighOnly(970.0, 1000.0);
                    cfg.radiationCpm = ThresholdBand::HighOnly(40.0, 80.0);
                    break;
                }
                case EnvironmentProfile::Space: {
                    //more controlled ECLSS-style environment
                    cfg.temperatureC = ThresholdBand::Range(19.0, 17.0, 25.0, 27.0);
                    cfg.humidityRH = ThresholdBand::Range(35.0, 25.0, 60.0, 70.0);

                    //ISS cabin pressure = 1013 hPa (sea-level equivalent)
                    cfg.pressureHpa = ThresholdBand::Range(1000.0, 980.0, 1025.0, 1040.0);

                    //NASA aims for tighter CO2 control than buildings
                    cfg.co2Ppm = ThresholdBand::HighOnly(2000.0, 3000.0);

                    //radiation in spacecraft is much more critical - need to mpdify once I get rad detector.
                    cfg.radiationCpm = ThresholdBand::HighOnly(20.0, 40.0);
                    break;
                }
            }
            return cfg;
        }
    };
    //converts health status enum to readable text
    inline const char* toString(HealthStatus s) {
        switch (s) {
        case HealthStatus::Nominal: return "Nominal";
        case HealthStatus::Warning: return "Warning";
        case HealthStatus::Critical: return "Critical";
            //just incase of an error.
        default: return "Unknown";
        }
    }

} 