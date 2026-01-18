/*
 * TEST 1: I2C Scanner
 * Run this FIRST to check if OLED and ADS1115 are detected
 * 
 * Expected output:
 *   0x3C = OLED Display
 *   0x48 = ADS1115
 */

#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  
  Serial.println("\n=============================");
  Serial.println("   I2C SCANNER TEST");
  Serial.println("=============================\n");
}

void loop() {
  byte error, address;
  int deviceCount = 0;
  
  Serial.println("Scanning I2C bus...\n");
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("FOUND: 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      if (address == 0x3C) Serial.print(" --> OLED Display");
      else if (address == 0x3D) Serial.print(" --> OLED Display (alt)");
      else if (address == 0x48) Serial.print(" --> ADS1115");
      else if (address == 0x49) Serial.print(" --> ADS1115 (alt)");
      
      Serial.println();
      deviceCount++;
    }
  }
  
  Serial.println();
  if (deviceCount == 0) {
    Serial.println("ERROR: No I2C devices found!");
    Serial.println("Check wiring:");
    Serial.println("  SDA -> GPIO 21");
    Serial.println("  SCL -> GPIO 22");
    Serial.println("  VCC -> 3.3V");
    Serial.println("  GND -> GND");
  } else {
    Serial.print("Total devices: ");
    Serial.println(deviceCount);
  }
  
  Serial.println("\n-----------------------------\n");
  delay(5000);
}
