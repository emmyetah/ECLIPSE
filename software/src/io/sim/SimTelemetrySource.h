#pragma once

#include <string>
#include <optional>
#include <cstdint>
#include <chrono>
#include <random>

#include "../config/Config.h"   // SimTelemetryConfig (adjust include path if yours differs)

namespace eclipse::io
{
    /**
     * Generates fake telemetry lines (CSV) at a fixed updateHz.
     * Useful for UI + pipeline testing without hardware.
     *
     * Output: raw CSV line (no trailing '\n')
     * Matches TelemetryParser expected order:
    */

    class SimTelemetrySource
    {
    public:

        explicit SimTelemetrySource(const SimTelemetryConfig& cfg);

        bool open();
        void close();
        bool isOpen() const;

        //returns a line only when the next simulated tick is due
        std::optional<std::string> pollLine();

    private:
        //stores telemetry data
        SimTelemetryConfig _cfg;
        //checks if 'active'
        bool _open = false;

        //timing ocntrol to simulae sensor update frequency.
        std::chrono::steady_clock::time_point _start{};
        std::chrono::steady_clock::time_point _nextTick{};

        //mersenne twister engine. known for efficiency 
        std::mt19937_64 _rng{ 123456789ULL };
        //noise model, generates gaussian noise. Decided to use Gaussian Noise to fake ransom spikes.
        std::normal_distribution<double> _noise{ 0.0, 0.15 };

        // helpers
        
        double noise();
        //ensures values stay in physicak bounds.
        static double clamp(double v, double lo, double hi);
        static std::string formatCsv(
            int64_t tsMs,
            double bmeTempC,
            double bmeRh,
            double bmePressureHpa,
            double scdCo2Ppm,
            double scdTempC,
            double scdRh,
            int64_t radCpm);
    };
}