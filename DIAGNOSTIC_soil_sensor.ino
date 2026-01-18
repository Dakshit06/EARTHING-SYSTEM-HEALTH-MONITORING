/*
 * SOIL SENSOR DIAGNOSTIC TEST
 * 
 * This will show if your soil sensor is connected correctly.
 * Open Serial Monitor at 115200 baud.
 * 
 * WIRING CHECK:
 * - Soil Sensor VCC → 3.3V
 * - Soil Sensor GND → GND  
 * - Soil Sensor AO  → ADS1115 A0
 */

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(21, 22);
  
  Serial.println("\n============================");
  Serial.println("  SOIL SENSOR DIAGNOSTIC");
  Serial.println("============================\n");
  
  if (!ads.begin(0x48)) {
    Serial.println("ERROR: ADS1115 not found!");
    Serial.println("Check I2C wiring");
    while(1);
  }
  
  Serial.println("ADS1115: OK\n");
  Serial.println("Reading all 4 channels...");
  Serial.println("Touch soil sensor probe to see if A0 changes!\n");
}

void loop() {
  int16_t a0 = ads.readADC_SingleEnded(0);  // Soil sensor
  int16_t a1 = ads.readADC_SingleEnded(1);  // Voltage sensor
  int16_t a2 = ads.readADC_SingleEnded(2);  // Not used
  int16_t a3 = ads.readADC_SingleEnded(3);  // Not used
  
  Serial.println("─────────────────────────────");
  Serial.print("A0 (Soil):    "); Serial.println(a0);
  Serial.print("A1 (Voltage): "); Serial.println(a1);
  Serial.print("A2 (unused):  "); Serial.println(a2);
  Serial.print("A3 (unused):  "); Serial.println(a3);
  
  // Check if A0 looks like sensor data
  if (a0 < 100) {
    Serial.println("\n⚠ A0 is very LOW - Sensor may not be connected!");
  } else if (a0 > 30000) {
    Serial.println("\n⚠ A0 is very HIGH - Check GND connection!");
  } else {
    Serial.println("\n✓ A0 reading looks OK");
  }
  
  Serial.println();
  delay(1000);
}

/*
 * EXPECTED RESULTS:
 * 
 * If sensor connected correctly:
 *   A0 should show 5000-20000 and CHANGE when you touch probe
 * 
 * If A0 stays same (~10000-11000):
 *   Sensor AO wire not connected to ADS1115 A0
 * 
 * If A0 shows 0 or very low:
 *   Sensor VCC not getting power (3.3V)
 * 
 * If A0 shows 32767:
 *   Sensor GND not connected
 */
