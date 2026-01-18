/*
 * Earthing Project - Soil Sensor Module
 * 
 * This code reads soil moisture/conductivity to monitor 
 * ground earthing quality for safety systems.
 * 
 * Components:
 * - Arduino Uno/Nano
 * - Soil Moisture Sensor (Capacitive or Resistive)
 * - 16x2 LCD Display (optional)
 * - Buzzer for alerts
 * - LED indicators (Red, Yellow, Green)
 */

// Pin Definitions
#define SOIL_SENSOR_PIN A0      // Analog pin for soil sensor
#define BUZZER_PIN 8            // Digital pin for buzzer
#define LED_RED 5               // Poor earthing indicator
#define LED_YELLOW 6            // Moderate earthing indicator
#define LED_GREEN 7             // Good earthing indicator

// Threshold values (adjust based on calibration)
#define THRESHOLD_POOR 300      // Below this = poor earthing
#define THRESHOLD_MODERATE 600  // Below this = moderate earthing
// Above THRESHOLD_MODERATE = good earthing

// Variables
int soilValue = 0;
int soilPercent = 0;
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 1000; // Read every 1 second

void setup() {
  // Initialize Serial Communication
  Serial.begin(9600);
  Serial.println("=================================");
  Serial.println("Earthing Project - Soil Sensor");
  Serial.println("Ground Monitoring System");
  Serial.println("=================================");
  
  // Initialize LED pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initial LED test
  testLEDs();
  
  Serial.println("System Ready!");
  Serial.println("");
}

void loop() {
  // Read sensor at defined intervals
  if (millis() - lastReadTime >= READ_INTERVAL) {
    lastReadTime = millis();
    
    // Read soil sensor value
    soilValue = readSoilSensor();
    
    // Convert to percentage (0-100%)
    soilPercent = map(soilValue, 0, 1023, 0, 100);
    
    // Display readings
    displayReadings();
    
    // Update status indicators
    updateStatus();
  }
}

int readSoilSensor() {
  // Take multiple readings for accuracy
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(SOIL_SENSOR_PIN);
    delay(10);
  }
  return total / 10;
}

void displayReadings() {
  Serial.print("Soil Sensor Value: ");
  Serial.print(soilValue);
  Serial.print(" | Conductivity: ");
  Serial.print(soilPercent);
  Serial.print("% | Status: ");
  
  if (soilValue < THRESHOLD_POOR) {
    Serial.println("POOR EARTHING - CHECK CONNECTION!");
  } else if (soilValue < THRESHOLD_MODERATE) {
    Serial.println("MODERATE EARTHING - Acceptable");
  } else {
    Serial.println("GOOD EARTHING - Optimal");
  }
}

void updateStatus() {
  // Turn off all LEDs first
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  
  if (soilValue < THRESHOLD_POOR) {
    // Poor earthing - Red LED + Buzzer alert
    digitalWrite(LED_RED, HIGH);
    alertBuzzer();
  } else if (soilValue < THRESHOLD_MODERATE) {
    // Moderate earthing - Yellow LED
    digitalWrite(LED_YELLOW, HIGH);
  } else {
    // Good earthing - Green LED
    digitalWrite(LED_GREEN, HIGH);
  }
}

void alertBuzzer() {
  // Sound buzzer for poor earthing warning
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void testLEDs() {
  Serial.println("Testing LEDs...");
  
  digitalWrite(LED_RED, HIGH);
  delay(300);
  digitalWrite(LED_RED, LOW);
  
  digitalWrite(LED_YELLOW, HIGH);
  delay(300);
  digitalWrite(LED_YELLOW, LOW);
  
  digitalWrite(LED_GREEN, HIGH);
  delay(300);
  digitalWrite(LED_GREEN, LOW);
  
  Serial.println("LED Test Complete!");
}

/*
 * CALIBRATION NOTES:
 * 
 * 1. Place sensor in dry soil - note the reading (typically 0-300)
 * 2. Place sensor in wet soil - note the reading (typically 600-1023)
 * 3. Adjust THRESHOLD_POOR and THRESHOLD_MODERATE accordingly
 * 
 * For earthing applications:
 * - Higher conductivity (wet soil) = Better earthing
 * - Lower conductivity (dry soil) = Poor earthing
 * 
 * WIRING:
 * Soil Sensor VCC  -> Arduino 5V
 * Soil Sensor GND  -> Arduino GND
 * Soil Sensor AO   -> Arduino A0
 * 
 * Red LED    -> Pin 5 (with 220Ω resistor)
 * Yellow LED -> Pin 6 (with 220Ω resistor)
 * Green LED  -> Pin 7 (with 220Ω resistor)
 * Buzzer     -> Pin 8
 */
