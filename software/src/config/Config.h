#pragma once

#include <cstdint>
#include <string>

#include "ThresholdConfig.h"

namespace eclipse {

    //defines where the data is coming from, fake or real ESP32 hardware. storing in 1 byte.
    enum class TelemetrySourceType : uint8_t {
        Serial = 0,
        Simulated = 1
    };

    //stores settings required to  connect to hardware (checked ESPRESSIF docs)
    struct SerialTelemetryConfig {
        std::string portName = "COM"; // normal for windows will update when reading from sensors if different
        uint32_t baudRate = 115200; //common for ESP32
    };

    //controls sim behaviour, may expand later
    struct SimTelemetryConfig {
        double updateHz = 2.0; //generated fake telemtry twice per second
    };

    //bundles everything about telemtry into one object
    struct TelemetryConfig {
        TelemetrySourceType sourceType = TelemetrySourceType::Serial;
        SerialTelemetryConfig serial;
        SimTelemetryConfig sim;
    };

    //basic info about my application
    struct AppConfig {
        //General
        std::string appName = "ECLIPSE";
        uint32_t samplePeriodMs = 1000; //how often i process telemetry 

        //Telemetry ( nested config)
        TelemetryConfig telemetry;

        // Thresholds
        ThresholdConfig thresholds = ThresholdConfig::defaults();

        static AppConfig defaults() {
            return AppConfig{};
        }
    };
} 