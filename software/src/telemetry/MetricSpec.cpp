#include "MetricSpec.h"

namespace eclipse::telemetry
{
    //safe fallback spec returned when an invalid metricId is requested.
    static const MetricSpec kInvalidSpec{ //static storage duration because we return by reference.
        MetricId::invalid,          //id
        "Invalid",                  //displayName
        core::units::Unit::none,    //unit
        0,                          //decimals
        0.0,                        //displayMin 
        0.0,                        //displayMax 
        255                         //sortOrder (push invalid to end)
    };

    //table of metric specifications
    static const MetricSpec kSpecs[] = {

        // -------- Raw Sensor Channels --------

        {
            MetricId::TempC_BME680,
            "Temp (BME680)",
            core::units::Unit::celsius,
            2,
            0.0,
            40.0,
            10 // gives space in between fused and raw metrics.
        },
        {
            MetricId::TempC_SCD30,
            "Temp (SCD30)",
            core::units::Unit::celsius,
            2,
            0.0,
            40.0,
            11
        },
        {
            MetricId::HumidityRH_BME680,
            "Humidity (BME680)",
            core::units::Unit::percent,
            1,
            0.0,
            100.0,
            12
        },
        {
            MetricId::HumidityRH_SCD30,
            "Humidity (SCD30)",
            core::units::Unit::percent,
            1,
            0.0,
            100.0,
            13
        },

        // -------- Fused / Official Metrics --------

        {
            MetricId::TempC,
            "Temperature",
            core::units::Unit::celsius,
            2,
            0.0,
            40.0,
            0
        },
        {
            MetricId::HumidityRH,
            "Humidity",
            core::units::Unit::percent,
            1,
            0.0,
            100.0,
            1
        },

        // -------- Other Metrics --------

        {
            MetricId::PressureHpa,
            "Pressure",
            core::units::Unit::hectopascal,
            1,
            900.0,
            1100.0,
            2
        },
        {
            MetricId::CO2ppm,
            "CO2",
            core::units::Unit::ppm,
            0,
            0.0,
            5000.0,
            3
        },
        {
            MetricId::RadiationCpm,
            "Radiation",
            core::units::Unit::cpm,
            0,
            0.0,
            2000.0,
            4
        }
    };

    const MetricSpec& GetMetricSpec(MetricId id)
    {
        //if id is invalid or sentinel, return the safe invalid spec.
        if (id == MetricId::invalid || id == MetricId::Count)
            return kInvalidSpec;

        //checking if spec is present in kSpecs
        for (const auto& spec : kSpecs)
        {
            if (spec.id == id)
                //returning if found.
                return spec;
        }

        //unknown id: return safe invalid spec.
        return kInvalidSpec;
    }

}