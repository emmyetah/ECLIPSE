#pragma once

#include <string>
#include <optional>
#include <cstdint>

#include "../../config/Config.h"

namespace eclipse::io
{
    /**
     * Reads telemetry lines from a real serial port (ESP32).
     * Output: raw CSV line (no trailing '\n')
     * Expected to match TelemetryParser format.
     */
    class SerialTelemetrySource
    {
    public:
        
        //explicit becuase it has one param and is used as a conversion
        explicit SerialTelemetrySource(const SerialTelemetryConfig& cfg);

        bool open();
        void close();
        bool isOpen() const;

        //returns a line if available, otherwise std::nullopt
        std::optional<std::string> pollLine();

        //need a deconstructor otherwise will leak as I have a raw pointer.
        ~SerialTelemetrySource();
        SerialTelemetrySource(const SerialTelemetrySource&) = delete;
        SerialTelemetrySource& operator=(const SerialTelemetrySource&) = delete;

    private:
        //configuration for serial telemetry
        SerialTelemetryConfig _cfg;

        //pointer to implementation
        struct Impl; // forward declaration only
        Impl* _impl = nullptr; //pointer only
    };
}