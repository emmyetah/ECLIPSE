#include <Arduino.h>
#include "src/sensors/SensorManager.h"

SensorManager sensorManager;

void setup() {
    Serial.begin(115200);
    delay(2000); // give serial time to connect

    if (!sensorManager.begin()) {
        // still continue so the Qt app can receive something predictable
    }
}

void loop() {
    SensorSnapshot snap = sensorManager.readAll();

    // timestamp
    Serial.print(snap.timestampMs);
    Serial.print(",");

    // BME680 temperature
    if (snap.bme680.valid) {
        Serial.print(snap.bme680.temperatureC, 2);
    } else {
        Serial.print("NA");
    }
    Serial.print(",");

    // BME680 humidity
    if (snap.bme680.valid) {
        Serial.print(snap.bme680.humidityRH, 2);
    } else {
        Serial.print("NA");
    }
    Serial.print(",");

    // BME680 pressure
    if (snap.bme680.valid) {
        Serial.print(snap.bme680.pressurehPa, 2);
    } else {
        Serial.print("NA");
    }
    Serial.print(",");

    // SCD30 CO2
    if (snap.scd30.valid) {
        Serial.print(snap.scd30.co2ppm, 2);
    } else {
        Serial.print("NA");
    }
    Serial.print(",");

    // SCD30 temperature
    if (snap.scd30.valid) {
        Serial.print(snap.scd30.temperatureC, 2);
    } else {
        Serial.print("NA");
    }
    Serial.print(",");

    // SCD30 humidity
    if (snap.scd30.valid) {
        Serial.print(snap.scd30.humidityRH, 2);
    } else {
        Serial.print("NA");
    }
    Serial.print(",");

    // Radiation placeholder for now
    Serial.println(0);

    delay(2000); // read every 2 seconds
}