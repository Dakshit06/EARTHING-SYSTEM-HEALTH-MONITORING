/*
 * TEST 3: ADS1115 ADC Test
 * Reads all 4 channels and displays raw values
 * 
 * Connect sensors:
 *   A0 = Soil Sensor AO
 *   A1 = Voltage Sensor OUT
 *   A2, A3 = unused (will show noise)
 */

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(21, 22);
  
  Serial.println("\n=============================");
  Serial.println("   ADS1115 ADC TEST");
  Serial.println("=============================\n");
  
  if (!ads.begin(0x48)) {
    Serial.println("ADS1115 FAILED!");
    Serial.println("Check wiring and address");
    while (1);
  }
  
  Serial.println("ADS1115 OK!");
  ads.setGain(GAIN_ONE);  // ±4.096V range
  Serial.println("Gain set to GAIN_ONE (±4.096V)\n");
}

void loop() {
  int16_t adc0 = ads.readADC_SingleEnded(0);  // Soil sensor
  int16_t adc1 = ads.readADC_SingleEnded(1);  // Voltage sensor
  int16_t adc2 = ads.readADC_SingleEnded(2);  // Unused
  int16_t adc3 = ads.readADC_SingleEnded(3);  // Unused
  
  // Convert to voltage (for reference)
  float volt0 = adc0 * 0.000125;  // 0.125mV per bit at GAIN_ONE
  float volt1 = adc1 * 0.000125;
  
  Serial.println("--- Readings ---");
  Serial.print("A0 (Soil):    Raw="); Serial.print(adc0);
  Serial.print("  Voltage="); Serial.print(volt0, 3); Serial.println("V");
  
  Serial.print("A1 (Voltage): Raw="); Serial.print(adc1);
  Serial.print("  Voltage="); Serial.print(volt1, 3); Serial.println("V");
  
  Serial.print("A2 (unused):  Raw="); Serial.println(adc2);
  Serial.print("A3 (unused):  Raw="); Serial.println(adc3);
  
  Serial.println();
  delay(1000);
}
