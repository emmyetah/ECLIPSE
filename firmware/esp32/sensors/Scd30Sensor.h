#pragma once
#include <Arduino.h>
#include <Adafruit_SCD30.h>

//basically the same as the BME680 header file - pressure.


//structure that holds the values of the scd30 reading and validates it.
struct Scd30Reading {
    float co2ppm = NAN;
    float temperatureC = NAN;
    float humidityRH = NAN;
    bool valid = false;
};

class Scd30Sensor {
public:
    bool begin(TwoWire& wire = Wire);
    Scd30Reading read();

private:
    Adafruit_SCD30 _scd30;
    bool _initialised = false;
};