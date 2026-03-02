#include <Arduino.h>
#include "src/sensors/SensorManager.h"

SensorManager sensorManager;

void setup() {
    Serial.begin(115200);
    delay(2000); // give serial time to connect

    Serial.println("Starting ECLIPSE Sensor Test...");

    if (!sensorManager.begin()) {
        Serial.println("⚠️ Sensor initialisation failed!");
    } else {
        Serial.println("✅ Sensors initialised successfully.");
    }
}

void loop() {
    SensorSnapshot snap = sensorManager.readAll();

    Serial.println("--------------------------------------------------");
    Serial.print("Timestamp (ms): ");
    Serial.println(snap.timestampMs);

    // ===== BME680 =====
    if (snap.bme680.valid) {
        Serial.println("BME680:");
        Serial.print("  Temp (C): ");
        Serial.println(snap.bme680.temperatureC);
        Serial.print("  Humidity (%): ");
        Serial.println(snap.bme680.humidityRH);
        Serial.print("  Pressure (hPa): ");
        Serial.println(snap.bme680.pressurehPa);
        Serial.print("  Gas (Ohms): ");
        Serial.println(snap.bme680.gasOhms);
    } else {
        Serial.println("BME680: ❌ Invalid reading");
    }

    // ===== SCD30 =====
    if (snap.scd30.valid) {
        Serial.println("SCD30:");
        Serial.print("  CO2 (ppm): ");
        Serial.println(snap.scd30.co2ppm);
        Serial.print("  Temp (C): ");
        Serial.println(snap.scd30.temperatureC);
        Serial.print("  Humidity (%): ");
        Serial.println(snap.scd30.humidityRH);
    } else {
        Serial.println("SCD30: ⏳ No new data yet");
    }

    // ===== Radiation =====
    //will add in later

    delay(2000);  // read every 2 seconds
}