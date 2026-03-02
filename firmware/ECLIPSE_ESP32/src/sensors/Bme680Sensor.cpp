#include "Bme680Sensor.h"


bool Bme680Sensor::begin(TwoWire& wire, uint8_t address) {
    //attempt to initialise the hardware dirver, (&wire gives memory address of the wire object)
    if (!_bme.begin(address, &wire)) {
        return false;
    }

    //once passed validation, get 
    // 
    //the argumets set how many internal measurements are averaged for each data type.

    _bme.setTemperatureOversampling(BME680_OS_8X);
    _bme.setHumidityOversampling(BME680_OS_2X);
    _bme.setPressureOversampling(BME680_OS_4X);
    //filter size prevents jitter in my GUI graphs
    _bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    //sets heater temperature to 320 degress for 150ms, this is needed to measure gas resistance properly.
    _bme.setGasHeater(320, 150);

    _initialised = true;
    //signal a successful setup
    return true;
}

//returns a structured telemetry object.
Bme680Reading Bme680Sensor::read() {
    //create a new struct. valies are set to NAN by default.
    Bme680Reading r;

    //if not initialised, return struct with NAN values
    if (!_initialised) return r;
    //triggerss measurement waits for completion (basiaclly an initial check.) 
    //if that diesn't work again return  struct with NAN values
    if (!_bme.performReading()) return r;

    //data extraction
    r.temperatureC = _bme.temperature;
    r.humidityRH = _bme.humidity;
    //sensor gives pascals, hectopascals (more human readable)
    r.pressurehPa = _bme.pressure / 100.0;
    r.gasOhms = _bme.gas_resistance;
    r.valid = true;
    //return the data struct.
    return r;
}