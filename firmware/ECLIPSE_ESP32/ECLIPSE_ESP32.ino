#include <Arduino.h>
#include "src/sensors/SensorManager.h"

SensorManager sensorManager;

// ---------------- Geiger settings ----------------
const int GEIGER_PIN = 27;

// Telemetry output every 2 seconds
const unsigned long TELEMETRY_INTERVAL_MS = 2000;

// Radiation updates every 30 seconds
const unsigned long RADIATION_INTERVAL_MS = 30000;

volatile unsigned long geigerPulses = 0;

unsigned long lastTelemetryTime = 0;
unsigned long lastRadiationUpdate = 0;

unsigned long radiationCPM = 0;        // value printed to ECLIPSE
unsigned long previousRadiationCPM = 0;
unsigned long lastRadiationWindowPulses = 0;

void IRAM_ATTR onGeigerPulse() {
    geigerPulses++;
}

void updateRadiationCPM() {
    noInterrupts();
    unsigned long pulses = geigerPulses;
    geigerPulses = 0;
    interrupts();

    lastRadiationWindowPulses = pulses;

    unsigned long currentCPM = (pulses * 60000UL) / RADIATION_INTERVAL_MS;

    // Only update displayed radiation value if it changed
    if (currentCPM != previousRadiationCPM) {
        radiationCPM = currentCPM;
        previousRadiationCPM = currentCPM;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    sensorManager.begin();

    pinMode(GEIGER_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(GEIGER_PIN), onGeigerPulse, RISING);

    lastTelemetryTime = millis();
    lastRadiationUpdate = millis();
}

void loop() {
    unsigned long now = millis();

    if (now - lastRadiationUpdate >= RADIATION_INTERVAL_MS) {
        updateRadiationCPM();
        lastRadiationUpdate = now;
    }

    if (now - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
        lastTelemetryTime = now;

        SensorSnapshot snap = sensorManager.readAll();

        Serial.print(snap.timestampMs);
        Serial.print(",");

        if (snap.bme680.valid) Serial.print(snap.bme680.temperatureC, 2);
        else Serial.print("NA");
        Serial.print(",");

        if (snap.bme680.valid) Serial.print(snap.bme680.humidityRH, 2);
        else Serial.print("NA");
        Serial.print(",");

        if (snap.bme680.valid) Serial.print(snap.bme680.pressurehPa, 2);
        else Serial.print("NA");
        Serial.print(",");

        if (snap.scd30.valid) Serial.print(snap.scd30.co2ppm, 2);
        else Serial.print("NA");
        Serial.print(",");

        if (snap.scd30.valid) Serial.print(snap.scd30.temperatureC, 2);
        else Serial.print("NA");
        Serial.print(",");

        if (snap.scd30.valid) Serial.print(snap.scd30.humidityRH, 2);
        else Serial.print("NA");
        Serial.print(",");

        Serial.println(radiationCPM);
    }
}