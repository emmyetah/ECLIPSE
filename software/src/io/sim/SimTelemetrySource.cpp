// SimTelemetrySource.cpp
#include "SimTelemetrySource.h"

#include <sstream>
#include <iomanip> //for formatting decimals

namespace eclipse::io
{
    //constructor
    SimTelemetrySource::SimTelemetrySource(const SimTelemetryConfig& cfg)
        : _cfg(cfg)
    {
    }

    //opens source, starts timer 
    bool SimTelemetrySource::open()
    {
        _open = true;
        _start = std::chrono::steady_clock::now();
        _nextTick = _start; // allow immediate first line
        return true;
    }

    //closes source
    void SimTelemetrySource::close()
    {
        _open = false;
    }

    //checks if source is active & open. returns bool
    bool SimTelemetrySource::isOpen() const
    {
        return _open;
    }

    //helper to generate gaussian nosie
    double SimTelemetrySource::noise()
    {
        //if conifg noise is invalid force it to 0, prevents weird math
        const double stddev = (_cfg.noiseStd <= 0.0) ? 0.0 : _cfg.noiseStd;
        //creates gaussian distribution and returns one random sample.
        std::normal_distribution<double> n(0.0, stddev);
        return n(_rng);
    }

    double SimTelemetrySource::unitRand()
    {
        //generate random 0,1 for spikes
        std::uniform_real_distribution<double> u(0.0, 1.0);
        return u(_rng);
    }

    
    std::optional<std::string> SimTelemetrySource::pollLine()
    {
        //open source/start simulation
        if (!_open) return std::nullopt;

        //get currne time.
        const auto now = std::chrono::steady_clock::now();
        //if it's not time yet, return nothing.
        if (now < _nextTick) {
            return std::nullopt; // not time yet
        }

        //schedule next tick
        const double hz = (_cfg.updateHz <= 0.0) ? 1.0 : _cfg.updateHz;
        //calculate period between updates
        const auto period = std::chrono::duration<double>(1.0 / hz);
        //next allowed tick update time.
        _nextTick = now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(period);

        //timestamp in ms since open()
        const int64_t tsMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - _start).count();

        //Base values from config + jitter
        const double bmeTempC = _cfg.bmeTempC + noise(); //small jitter
        const double bmeRh = clamp(_cfg.bmeRhPct + (noise() * 2.0), 0.0, 100.0); //larger jitter, clamp between physical bounds, 0 and 100
        const double bmePressureHpa = clamp(_cfg.bmePressureHpa + (noise() * 1.5), 900.0, 1100.0); //clamping pressure and adding more noise

        const double scdCo2Ppm = clamp(_cfg.scdCo2Ppm + (noise() * 10.0), 350.0, 5000.0); //preasure fluctuates more noticeablly
        const double scdTempC = _cfg.scdTempC + noise(); //same again for temp, small jitter
        const double scdRh = clamp(_cfg.scdRhPct + (noise() * 2.0), 0.0, 100.0);

        const int64_t radCpm = static_cast<int64_t>(clamp(_cfg.radCpm + (noise() * 3.0), 0.0, 5000.0));

        //Apply “space mode” spikes
        if (_cfg.enableSpikes) {
            if (unitRand() < clamp(_cfg.spikeChance, 0.0, 1.0)) {
                scdCo2Ppm = clamp(scdCo2Ppm + _cfg.co2SpikePpm, 350.0, 5000.0);
                radCpm = static_cast<int64_t>(clamp((double)radCpm + _cfg.radSpikeCpm, 0.0, 5000.0));
            }
        }

        return formatCsv(
            tsMs,
            bmeTempC,
            bmeRh,
            bmePressureHpa,
            scdCo2Ppm,
            scdTempC,
            scdRh,
            radCpm
        );
    }

    //clamps a value between two numbers
    double SimTelemetrySource::clamp(double v, double lo, double hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    //
    std::string SimTelemetrySource::formatCsv(
        int64_t tsMs,
        double bmeTempC,
        double bmeRh,
        double bmePressureHpa,
        double scdCo2Ppm,
        double scdTempC,
        double scdRh,
        int64_t radCpm)
    {
        // format: ts_ms,bme_temp_c,bme_rh_pct,bme_pressure_hpa,scd_co2_ppm,scd_temp_c,scd_rh_pct,rad_cpm
        std::ostringstream out;
        out << tsMs << ','
            << std::fixed << std::setprecision(2)
            << bmeTempC << ','
            << bmeRh << ','
            << bmePressureHpa << ','
            << std::setprecision(1)
            << scdCo2Ppm << ','
            << std::setprecision(2)
            << scdTempC << ','
            << scdRh << ','
            << radCpm;

        return out.str();
    }
}