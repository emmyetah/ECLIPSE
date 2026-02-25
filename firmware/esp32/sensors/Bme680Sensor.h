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
    //even if values are present I only trust them is valid is true.
    bool valid = false;
};

class Bme680Sensor {
public:
    //function to make sure sensor is initialised correcttly.
    //- TwoWire is arduinos i2c bus object, & for no copies to avoid memory waste.
    bool begin(TwoWire& wire = Wire, 
        //unsigned int 8 bits (1 byte) reduces memory usage as I2C addresses are 1 byte long
        uint8_t address = 0x77); //collected from i2c scanner in arduino
    //reads sesor values and returns a structured data object.
    Bme680Reading read();

private:
    //The hardware driver object. (created instance of the bme680 driver class in adafruit)
    //did this for extra security, protection of data so that I'm not changing the hard data.
    //also for flexibility, I can present the bme680reading data any way that I'd like wihtout changing or affecting the REAL data.
    Adafruit_BME680 _bme;
    bool _initialised = false;
};