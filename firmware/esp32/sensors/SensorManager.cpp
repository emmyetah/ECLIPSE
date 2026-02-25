#include "SensorManager.h"
#include <Wire.h>

//same as the individual sesnor begin fucnitons, returns bool for validation.
bool SensorManager::begin() {
    //configures SDA & SCL pins, sets clock speed and enables i2C controller.
    Wire.begin();

    bool ok = true;
    //put & to avoid overwriting failure state. preserves failure state if somehting goes worng
    //- Professional embeded pattern.
    ok &= _bme680.begin(Wire, 0x77);
    ok &= _scd30.begin(Wire); // address not reuqired because scd30 has fixed address of 61
    // will add radiation sensor here 

    return ok;
}

//returns a full snapshop
SensorSnapshot SensorManager::readAll() {
    SensorSnapshot snap;

    snap.timestampMs = millis(); //returns ms since ESP32 boot.

    //I'll compare and avergae in the telemetry layer :)
    snap.bme680 = _bme680.read();
    snap.scd30 = _scd30.read();

    return snap;
}