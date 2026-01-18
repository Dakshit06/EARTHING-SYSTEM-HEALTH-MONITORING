/*
 * ============================================================
 * ESP32 SMART EARTHING MONITORING SYSTEM - COMPLETE CODE
 * ============================================================
 * 
 * Components: ESP32, ADS1115, 0.96" OLED, Soil Sensor, 
 *             Voltage Sensor, Buzzer, Red LED
 * 
 * Features:
 * - I2C Scanner (startup)
 * - Component testing (startup)
 * - Calibration mode display
 * - Earthing monitoring with alerts
 * 
 * Pin Configuration:
 * - GPIO 21 = SDA (I2C)
 * - GPIO 22 = SCL (I2C)
 * - GPIO 26 = Red LED (via 220Ω)
 * - GPIO 27 = Buzzer
 * - ADS1115 A0 = Soil Sensor
 * - ADS1115 A1 = Voltage Sensor
 * 
 * Author: Earthing Project
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== CONFIGURATION ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C      // Try 0x3D if not working
#define ADS_ADDRESS 0x48

#define SDA_PIN 21
#define SCL_PIN 22
#define BUZZER_PIN 27
#define LED_PIN 26

// ==================== CALIBRATION (ADJUST THESE!) ====================
#define SOIL_DRY_RAW 17000     // Raw value when soil is DRY
#define SOIL_WET_RAW 6000      // Raw value when soil is WET
#define SOIL_ALERT_PERCENT 30  // Alert when moisture <= 30%
#define VOLTAGE_THRESHOLD 500  // Alert when voltage <= this value

// ==================== OBJECTS ====================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_ADS1115 ads;

// ==================== VARIABLES ====================
bool oledOK = false;
bool adsOK = false;
int16_t soilRaw = 0;
int16_t voltageRaw = 0;
int soilPercent = 0;
bool soilFault = false;
bool voltageFault = false;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(100);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  ESP32 EARTHING MONITORING SYSTEM      ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // STEP 1: I2C Scan
  Serial.println("[TEST 1] I2C Scanner");
  scanI2C();
  
  // STEP 2: Initialize OLED
  Serial.println("[TEST 2] OLED Display");
  initOLED();
  
  // STEP 3: Initialize ADS1115
  Serial.println("[TEST 3] ADS1115 ADC");
  initADS();
  
  // STEP 4: Test LED & Buzzer
  Serial.println("[TEST 4] LED & Buzzer");
  testOutputs();
  
  // Show startup screen
  if (oledOK) {
    showStartup();
  }
  
  Serial.println("\n[READY] System monitoring started!\n");
  Serial.println("─────────────────────────────────────────\n");
}

// ==================== MAIN LOOP ====================
void loop() {
  // Read sensors
  readSensors();
  
  // Check faults
  soilFault = (soilPercent <= SOIL_ALERT_PERCENT);
  voltageFault = (voltageRaw <= VOLTAGE_THRESHOLD);
  
  // Update alerts
  if (soilFault || voltageFault) {
    digitalWrite(LED_PIN, HIGH);
    // Intermittent buzzer
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
  
  // Update display
  updateDisplay();
  
  // Serial debug
  Serial.print("Soil: ");
  Serial.print(soilRaw);
  Serial.print(" (");
  Serial.print(soilPercent);
  Serial.print("%) | Volt: ");
  Serial.print(voltageRaw);
  Serial.print(" | Status: ");
  Serial.println((soilFault || voltageFault) ? "⚠ ALERT" : "✓ OK");
  
  delay(2000);
}

// ==================== I2C SCANNER ====================
void scanI2C() {
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  → Found: 0x");
      Serial.print(addr, HEX);
      if (addr == 0x3C || addr == 0x3D) Serial.print(" (OLED)");
      if (addr == 0x48 || addr == 0x49) Serial.print(" (ADS1115)");
      Serial.println();
      found++;
    }
  }
  if (found == 0) {
    Serial.println("  ✗ No I2C devices! Check: SDA→21, SCL→22");
  }
  Serial.println();
}

// ==================== INIT OLED ====================
void initOLED() {
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    oledOK = true;
    Serial.println("  ✓ OLED OK");
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
  } else {
    Serial.println("  ✗ OLED FAILED (try 0x3D)");
  }
  Serial.println();
}

// ==================== INIT ADS1115 ====================
void initADS() {
  if (ads.begin(ADS_ADDRESS)) {
    adsOK = true;
    ads.setGain(GAIN_ONE);
    Serial.println("  ✓ ADS1115 OK");
  } else {
    Serial.println("  ✗ ADS1115 FAILED");
  }
  Serial.println();
}

// ==================== TEST OUTPUTS ====================
void testOutputs() {
  Serial.print("  Testing LED... ");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  Serial.println("✓");
  
  Serial.print("  Testing Buzzer... ");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("✓");
  Serial.println();
}

// ==================== STARTUP SCREEN ====================
void showStartup() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(15, 5);
  display.println("EARTHING MONITOR");
  display.drawLine(0, 16, 128, 16, SSD1306_WHITE);
  
  display.setCursor(0, 22);
  display.print("OLED:    "); display.println(oledOK ? "OK" : "FAIL");
  display.print("ADS1115: "); display.println(adsOK ? "OK" : "FAIL");
  display.print("LED:     OK");
  display.setCursor(0, 48);
  display.println("Buzzer:  OK");
  
  display.setCursor(20, 56);
  display.println("Starting...");
  display.display();
  delay(3000);
}

// ==================== READ SENSORS ====================
void readSensors() {
  if (!adsOK) {
    Serial.println("ERROR: ADS1115 not connected!");
    return;
  }
  
  // Take 5 readings and average for stability
  long soilTotal = 0;
  long voltTotal = 0;
  
  for (int i = 0; i < 5; i++) {
    soilTotal += ads.readADC_SingleEnded(0);
    voltTotal += ads.readADC_SingleEnded(1);
    delay(10);
  }
  
  soilRaw = soilTotal / 5;
  voltageRaw = voltTotal / 5;
  
  // DEBUG: Print raw before mapping
  Serial.print("DEBUG → Raw: ");
  Serial.print(soilRaw);
  Serial.print(" | Dry=");
  Serial.print(SOIL_DRY_RAW);
  Serial.print(" Wet=");
  Serial.print(SOIL_WET_RAW);
  
  soilPercent = map(soilRaw, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);
  
  Serial.print(" | Mapped %: ");
  Serial.println(soilPercent);
}

// ==================== UPDATE DISPLAY ====================
void updateDisplay() {
  if (!oledOK) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  
  // Header
  display.setCursor(10, 0);
  display.println("EARTHING MONITOR");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Soil moisture
  display.setCursor(0, 14);
  display.print("Soil: ");
  display.print(soilPercent);
  display.print("% ");
  display.print(soilFault ? "[LOW!]" : "[OK]");
  
  // Raw value (for calibration)
  display.setCursor(0, 24);
  display.print("(Raw: ");
  display.print(soilRaw);
  display.print(")");
  
  // Voltage
  display.setCursor(0, 36);
  display.print("Voltage: ");
  display.print(voltageRaw);
  display.print(" ");
  display.print(voltageFault ? "[FAULT]" : "[OK]");
  
  // Status line
  display.drawLine(0, 48, 128, 48, SSD1306_WHITE);
  display.setCursor(0, 52);
  
  if (soilFault || voltageFault) {
    display.setTextSize(1);
    if (soilFault) {
      display.println("! EARTHING NOT PROPER");
      display.setCursor(0, 62);
      display.print("Give Water to Earthing");
    } else {
      display.println("! VOLTAGE FAULT");
      display.setCursor(0, 62);
      display.print("Check Earth Connection");
    }
  } else {
    display.setTextSize(2);
    display.setCursor(5, 52);
    display.println("EARTHING OK");
  }
  
  display.display();
}

/*
 * ============================================================
 * CALIBRATION GUIDE
 * ============================================================
 * 
 * 1. Upload this code and open Serial Monitor (115200 baud)
 * 2. Check I2C devices are detected
 * 3. Put soil sensor in DRY soil - note the "Soil: xxxx" raw value
 * 4. Put soil sensor in WET soil - note the raw value
 * 5. Update these lines at the top:
 *    #define SOIL_DRY_RAW xxxxx
 *    #define SOIL_WET_RAW xxxxx
 * 6. For voltage sensor, disconnect input and note raw value,
 *    then connect and note value. Set VOLTAGE_THRESHOLD between them.
 * 
 * ============================================================
 * WIRING SUMMARY
 * ============================================================
 * 
 * ESP32       →  OLED      →  ADS1115
 * GPIO 21     →  SDA       →  SDA
 * GPIO 22     →  SCL       →  SCL
 * 3.3V        →  VDD       →  VDD
 * GND         →  GND       →  GND, ADDR
 * 
 * ADS1115     →  Sensors
 * A0          →  Soil Sensor AO
 * A1          →  Voltage Sensor OUT
 * 
 * ESP32       →  Outputs
 * GPIO 26     →  220Ω → LED (+)
 * GPIO 27     →  Buzzer (+)
 * GND         →  LED (-), Buzzer (-)
 * 
 * ============================================================
 */
