#include <Wire.h>

void setup() {
  Wire.begin();               // Start I2C
  Serial.begin(115200);       // Open serial communication
  while (!Serial);            // Wait for serial monitor
  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int devices = 0;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }

  if (devices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("Scan complete");

  delay(3000); // wait 3 seconds before next scan
}
