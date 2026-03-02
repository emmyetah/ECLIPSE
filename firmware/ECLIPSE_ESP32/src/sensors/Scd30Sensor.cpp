#include "Scd30Sensor.h"

bool Scd30Sensor::begin(TwoWire& wire, uint8_t addr) {

    if (!_scd30.begin(0x61, &wire)) {
        return false;
    }

    //tells the sensor to take a new measurement every 2 seconds 
    // (not needed for bme680, bme  gets triggered when we need the data scd30 doesn't)
    _scd30.setMeasurementInterval(2);  
    _initialised = true;
    return true;
}

Scd30Reading Scd30Sensor::read() {
    //same as bme680, creates and returns a struct object after validation.
    Scd30Reading r;

    if (!_initialised) return r;
    //we check if the new data is ready after the measurement interval. 
    if (!_scd30.dataReady()) return r;

    if (_scd30.read()) {
        r.co2ppm = _scd30.CO2;
        r.temperatureC = _scd30.temperature;
        r.humidityRH = _scd30.relative_humidity;
        r.valid = true;
    }

    return r;
}