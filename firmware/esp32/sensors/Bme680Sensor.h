#pragma once

//arduino core framework
#include <Arduino.h>
#include <Adafruit_BME680.h>

//struct for the data types collected.
struct Bme680Reading {
    float temperatureC = NAN; //Not a number, prevents corrupt telemtry form entering my pipeline.
    float humidityRH = NAN;
    float pressurehPa = NAN;
    float gasOhms = NAN;
    bool valid = false;
};

class Bme680Sensor {
public:
    bool begin(TwoWire& wire = Wire, 
        //unsigned int 8 bits (1 byte) reduces memory usage as I2C addresses are 1 byte long
        uint8_t address = 0x77); //collected from i2c scanner in arduino
    Bme680Reading read();

private:
    Adafruit_BME680 _bme;
    bool _initialised = false;
};