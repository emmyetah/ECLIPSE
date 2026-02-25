#pragma once

//Need to update to include geiger counter when it arrives and is configured

#include "Bme680Sensor.h"
#include "Scd30Sensor.h"

//captures complete environmental state of the system at a single moment in time
struct SensorSnapshot {
    //Milliseconds
    unsigned long timestampMs = 0;

    //data structs from each sensor
    Bme680Reading bme680;
    Scd30Reading scd30;
};

class SensorManager {
public:
    //initialises both sensors
    bool begin();
    //reads all sensors, populates te struct and returns the snapshot (averages sensors that have the same data type for validaiton)
    SensorSnapshot readAll();

    //HAL wrappers
private:
    Bme680Sensor _bme680;
    Scd30Sensor _scd30;
};