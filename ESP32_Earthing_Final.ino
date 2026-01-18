/*
 * ============================================================
 * ESP32 SMART EARTHING MONITORING SYSTEM
 * ============================================================
 * 
 * DIRECT ESP32 ADC - NO ADS1115 FOR SOIL SENSOR
 * 
 * Wiring:
 * - Soil Sensor AO → GPIO34 (Analog)
 * - Soil Sensor DO → GPIO35 (Digital)
 * - Voltage Sensor → ADS1115 A1 (if using)
 * - OLED SDA → GPIO21
 * - OLED SCL → GPIO22
 * - Buzzer → GPIO27
 * - LED → GPIO26
 * 
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== PIN CONFIGURATION ====================
#define SOIL_ANALOG_PIN   34    // Soil sensor AO → GPIO34
#define SOIL_DIGITAL_PIN  35    // Soil sensor DO → GPIO35
#define BUZZER_PIN        27
#define LED_PIN           26
#define SDA_PIN           21
#define SCL_PIN           22

// ==================== DISPLAY CONFIG ====================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDR     0x3C

// ==================== CALIBRATION ====================
// ESP32 ADC range: 0-4095 (12-bit)
// Capacitive sensor: HIGH = DRY, LOW = WET
#define SOIL_DRY_VALUE    2800   // Dry air/soil value (~2500-3000)
#define SOIL_WET_VALUE    1200   // Wet soil/water value (~1000-1500)
#define SOIL_ALERT_THRESHOLD 30  // Alert when moisture <= 30%

// ==================== OBJECTS ====================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==================== VARIABLES ====================
bool oledReady = false;
int soilAnalogRaw = 0;
int soilDigital = 0;
int soilPercent = 0;
bool earthingAlert = false;

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println();
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║  ESP32 EARTHING MONITORING SYSTEM    ║");
  Serial.println("║  Direct ADC Mode (No ADS1115)        ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  
  // Configure pins
  pinMode(SOIL_ANALOG_PIN, INPUT);   // GPIO34 - Analog input
  pinMode(SOIL_DIGITAL_PIN, INPUT);  // GPIO35 - Digital input
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // Configure ADC
  analogReadResolution(12);          // 12-bit resolution (0-4095)
  analogSetAttenuation(ADC_11db);    // Full range 0-3.3V
  
  // Initialize I2C for OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(100);
  
  // Initialize OLED
  Serial.print("[OLED] Initializing... ");
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    oledReady = true;
    Serial.println("OK");
    display.clearDisplay();
    display.display();
  } else {
    Serial.println("FAILED");
  }
  
  // Test outputs
  Serial.print("[LED] Testing... ");
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
  Serial.println("OK");
  
  Serial.print("[Buzzer] Testing... ");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(150);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("OK");
  
  // Test sensor reading
  Serial.println();
  Serial.println("[SENSOR] Testing soil sensor on GPIO34...");
  int testRead = analogRead(SOIL_ANALOG_PIN);
  Serial.print("   Initial reading: ");
  Serial.println(testRead);
  
  if (testRead > 0 && testRead < 4095) {
    Serial.println("   ✓ Sensor responding!");
  } else {
    Serial.println("   ⚠ Check sensor connection!");
  }
  
  Serial.println();
  Serial.println("========================================");
  Serial.println("MONITORING STARTED");
  Serial.println("Touch sensor probe to see values change!");
  Serial.println("========================================");
  Serial.println();
  
  // Show startup screen
  if (oledReady) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 20);
    display.println("SYSTEM READY");
    display.setCursor(20, 40);
    display.println("Starting...");
    display.display();
    delay(2000);
  }
}

// ============================================================
// MAIN LOOP
// ============================================================
void loop() {
  // Read soil sensor from ESP32 ADC
  readSoilSensor();
  
  // Check alert condition
  checkAlert();
  
  // Update display
  updateDisplay();
  
  // Serial debug
  printDebug();
  
  delay(1000);  // Update every 1 second
}

// ============================================================
// READ SOIL SENSOR
// ============================================================
void readSoilSensor() {
  // Take multiple readings and average for stability
  long total = 0;
  const int samples = 20;
  
  for (int i = 0; i < samples; i++) {
    total += analogRead(SOIL_ANALOG_PIN);
    delay(5);
  }
  
  soilAnalogRaw = total / samples;
  
  // Read digital output (threshold-based on sensor)
  soilDigital = digitalRead(SOIL_DIGITAL_PIN);
  
  // Calculate percentage
  // ESP32 ADC: DRY = higher value (~2500-3000), WET = lower value (~1000-1500)
  soilPercent = map(soilAnalogRaw, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);
}

// ============================================================
// CHECK ALERT
// ============================================================
void checkAlert() {
  earthingAlert = (soilPercent <= SOIL_ALERT_THRESHOLD);
  
  if (earthingAlert) {
    digitalWrite(LED_PIN, HIGH);
    // Short beep
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ============================================================
// UPDATE DISPLAY
// ============================================================
void updateDisplay() {
  if (!oledReady) return;
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Title
  display.setTextSize(1);
  display.setCursor(15, 0);
  display.println("EARTHING MONITOR");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Soil moisture percentage
  display.setCursor(0, 14);
  display.print("Moisture: ");
  display.print(soilPercent);
  display.print("% ");
  if (soilPercent <= SOIL_ALERT_THRESHOLD) {
    display.println("[LOW!]");
  } else {
    display.println("[OK]");
  }
  
  // Raw ADC value (for calibration)
  display.setCursor(0, 26);
  display.print("Raw ADC: ");
  display.println(soilAnalogRaw);
  
  // Digital output status
  display.setCursor(0, 38);
  display.print("Digital: ");
  display.println(soilDigital == HIGH ? "DRY" : "WET");
  
  // Status bar
  display.drawLine(0, 50, 128, 50, SSD1306_WHITE);
  
  display.setCursor(0, 54);
  if (earthingAlert) {
    display.println("! EARTHING NOT PROPER");
    display.setCursor(0, 64);
    display.print("Give Water!");
  } else {
    display.setTextSize(1);
    display.setCursor(25, 54);
    display.println("EARTHING OK");
  }
  
  display.display();
}

// ============================================================
// PRINT DEBUG
// ============================================================
void printDebug() {
  Serial.print("Raw: ");
  Serial.print(soilAnalogRaw);
  Serial.print(" | %: ");
  Serial.print(soilPercent);
  Serial.print(" | Digital: ");
  Serial.print(soilDigital == HIGH ? "DRY" : "WET");
  Serial.print(" | Alert: ");
  Serial.println(earthingAlert ? "YES ⚠" : "NO ✓");
}

/*
 * ============================================================
 * WIRING DIAGRAM
 * ============================================================
 *
 *   SOIL MOISTURE SENSOR          ESP32
 *   ────────────────────          ─────
 *   VCC ─────────────────────────► 3.3V
 *   GND ─────────────────────────► GND
 *   AO  ─────────────────────────► GPIO34 (Analog)
 *   DO  ─────────────────────────► GPIO35 (Digital)
 *
 *
 *   OLED DISPLAY                  ESP32
 *   ────────────                  ─────
 *   VCC ─────────────────────────► 3.3V
 *   GND ─────────────────────────► GND
 *   SDA ─────────────────────────► GPIO21
 *   SCL ─────────────────────────► GPIO22
 *
 *
 *   OUTPUTS                       ESP32
 *   ───────                       ─────
 *   Buzzer (+) ──────────────────► GPIO27
 *   Buzzer (-) ──────────────────► GND
 *   LED (+) ─────[220Ω]──────────► GPIO26
 *   LED (-) ─────────────────────► GND
 *
 *
 * ============================================================
 * CALIBRATION
 * ============================================================
 *
 * 1. Upload and open Serial Monitor (115200)
 *
 * 2. Put sensor in DRY AIR:
 *    - Note "Raw: XXXX" value (typically 2500-3000)
 *    - Update: #define SOIL_DRY_VALUE XXXX
 *
 * 3. Put sensor in WATER:
 *    - Note "Raw: XXXX" value (typically 1000-1500)
 *    - Update: #define SOIL_WET_VALUE XXXX
 *
 * 4. Re-upload and percentage will work correctly!
 *
 * ============================================================
 * ESP32 ADC NOTES
 * ============================================================
 *
 * GPIO34, GPIO35 are INPUT-ONLY pins (no internal pull-up)
 * ESP32 ADC range: 0-4095 (12-bit)
 * ADC_11db attenuation: 0-3.3V input range
 *
 * If readings are unstable:
 * - Increase number of samples in readSoilSensor()
 * - Add 100nF capacitor between sensor AO and GND
 *
 */
