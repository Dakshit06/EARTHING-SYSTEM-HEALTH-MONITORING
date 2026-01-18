/*
 * Smart Earthing Monitoring System using ESP32 and ADS1115
 * 
 * Components:
 * - ESP32 DevKit
 * - ADS1115 (I2C ADC)
 * - 0.96" I2C OLED Display (SSD1306)
 * - Soil Moisture Sensor
 * - Voltage Sensor Module
 * - Buzzer
 * - Red LED
 * 
 * Logic:
 * - If soil moisture ≤ 30% → Alert: "Earthing NOT Proper - Give Water to Earthing"
 * - If no voltage detected → Alert: "Earthing Fault"
 * - Both OK → "Earthing OK"
 */

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== OLED CONFIG ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C  // Try 0x3D if OLED doesn't work

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==================== ADS1115 CONFIG ====================
Adafruit_ADS1115 ads;
#define ADS_ADDRESS 0x48  // Default ADS1115 address

// ==================== ESP32 PIN CONFIG ====================
#define SDA_PIN 21
#define SCL_PIN 22
#define BUZZER_PIN 27
#define RED_LED_PIN 26

// ==================== THRESHOLD CONFIG ====================
#define SOIL_ALERT_PERCENT 30       // Alert if moisture ≤ 30%
#define VOLTAGE_THRESHOLD 500       // Adjust after testing

// ==================== CALIBRATION VALUES ====================
// IMPORTANT: Measure these values with Serial Monitor
// Put sensor in DRY soil → note the RAW value
// Put sensor in WET soil → note the RAW value
#define SOIL_DRY_RAW 17000   // Typical dry value (higher = drier)
#define SOIL_WET_RAW 6000    // Typical wet value (lower = wetter)

// ==================== VARIABLES ====================
bool oledOK = false;
bool adsOK = false;
int16_t soilRaw = 0;
int16_t voltageRaw = 0;
int soilPercent = 0;
bool soilFault = false;
bool voltageFault = false;
bool earthingFault = false;

// ==================== FUNCTION PROTOTYPES ====================
void scanI2C();
void initializeOLED();
void initializeADS();
void readSensors();
void updateAlerts();
void updateDisplay();
void displayMessage(String line1, String line2 = "", String line3 = "", String line4 = "");

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);  // Wait for serial
  
  Serial.println("\n================================");
  Serial.println("Smart Earthing Monitoring System");
  Serial.println("ESP32 + ADS1115 + OLED");
  Serial.println("================================\n");
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  
  // Initialize I2C with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(100);
  
  // Scan for I2C devices first
  Serial.println("[I2C] Scanning for devices...");
  scanI2C();
  
  // Initialize OLED
  initializeOLED();
  
  // Initialize ADS1115
  initializeADS();
  
  // Startup complete
  if (oledOK) {
    displayMessage("System Ready", "Monitoring...", "", "Earthing Monitor v1.0");
  }
  
  Serial.println("\n[SETUP] Complete! Starting monitoring...\n");
  delay(2000);
}

// ==================== MAIN LOOP ====================
void loop() {
  // Read sensors
  readSensors();
  
  // Update alerts (buzzer + LED)
  updateAlerts();
  
  // Update OLED display
  updateDisplay();
  
  // Debug output
  Serial.print("[DATA] Soil Raw: ");
  Serial.print(soilRaw);
  Serial.print(" | Soil %: ");
  Serial.print(soilPercent);
  Serial.print(" | Voltage Raw: ");
  Serial.print(voltageRaw);
  Serial.print(" | Fault: ");
  Serial.println(earthingFault ? "YES" : "NO");
  
  delay(2000);  // Read every 2 seconds
}

// ==================== I2C SCANNER ====================
void scanI2C() {
  byte error, address;
  int deviceCount = 0;
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("[I2C] Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // Identify common devices
      if (address == 0x3C || address == 0x3D) {
        Serial.println(" (OLED Display)");
      } else if (address == 0x48) {
        Serial.println(" (ADS1115)");
      } else {
        Serial.println();
      }
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("[I2C] ERROR: No devices found!");
    Serial.println("[I2C] Check wiring:");
    Serial.println("      SDA -> GPIO 21");
    Serial.println("      SCL -> GPIO 22");
    Serial.println("      VCC -> 3.3V");
    Serial.println("      GND -> GND");
  } else {
    Serial.print("[I2C] Found ");
    Serial.print(deviceCount);
    Serial.println(" device(s)");
  }
  Serial.println();
}

// ==================== INITIALIZE OLED ====================
void initializeOLED() {
  Serial.print("[OLED] Initializing at 0x");
  Serial.print(OLED_ADDRESS, HEX);
  Serial.print("... ");
  
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    oledOK = true;
    Serial.println("OK");
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED OK");
    display.display();
  } else {
    oledOK = false;
    Serial.println("FAILED!");
    Serial.println("[OLED] Try changing OLED_ADDRESS to 0x3D");
  }
}

// ==================== INITIALIZE ADS1115 ====================
void initializeADS() {
  Serial.print("[ADS1115] Initializing at 0x");
  Serial.print(ADS_ADDRESS, HEX);
  Serial.print("... ");
  
  if (ads.begin(ADS_ADDRESS)) {
    adsOK = true;
    Serial.println("OK");
    
    // Set gain (adjust sensitivity)
    // GAIN_ONE = ±4.096V (1 bit = 0.125mV)
    ads.setGain(GAIN_ONE);
  } else {
    adsOK = false;
    Serial.println("FAILED!");
    Serial.println("[ADS1115] Check wiring and address");
  }
}

// ==================== READ SENSORS ====================
void readSensors() {
  if (!adsOK) {
    Serial.println("[ERROR] ADS1115 not available!");
    return;
  }
  
  // Read soil sensor from ADS1115 channel A0
  soilRaw = ads.readADC_SingleEnded(0);
  
  // Read voltage sensor from ADS1115 channel A1
  voltageRaw = ads.readADC_SingleEnded(1);
  
  // Convert soil raw to percentage
  // map(value, fromLow, fromHigh, toLow, toHigh)
  // DRY = 0%, WET = 100%
  soilPercent = map(soilRaw, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);  // Keep within 0-100
  
  // Determine fault conditions
  soilFault = (soilPercent <= SOIL_ALERT_PERCENT);
  voltageFault = (voltageRaw <= VOLTAGE_THRESHOLD);
  earthingFault = soilFault || voltageFault;
}

// ==================== UPDATE ALERTS ====================
void updateAlerts() {
  if (earthingFault) {
    // Turn ON buzzer and LED
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
  } else {
    // Turn OFF buzzer and LED
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
  }
}

// ==================== UPDATE DISPLAY ====================
void updateDisplay() {
  if (!oledOK) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Line 1-2: Soil moisture status
  display.setCursor(0, 0);
  if (soilFault) {
    display.println("! SOIL MOISTURE LOW !");
  } else {
    display.println("Soil Moisture: OK");
  }
  
  display.setCursor(0, 10);
  display.print("Moisture: ");
  display.print(soilPercent);
  display.println("%");
  
  // Line 3: Voltage status
  display.setCursor(0, 24);
  if (voltageFault) {
    display.println("! NO VOLTAGE !");
  } else {
    display.println("Voltage: Present");
  }
  
  // Separator line
  display.drawLine(0, 35, 128, 35, SSD1306_WHITE);
  
  // Line 4-5: Earthing status (main message)
  display.setCursor(0, 40);
  if (earthingFault) {
    display.setTextSize(1);
    display.println("EARTHING NOT PROPER");
    display.setCursor(0, 52);
    if (soilFault) {
      display.println("Give Water to Earthing");
    } else {
      display.println("Check Earthing Fault");
    }
  } else {
    display.setTextSize(2);
    display.setCursor(10, 44);
    display.println("EARTHING OK");
  }
  
  display.display();
}

// ==================== DISPLAY MESSAGE HELPER ====================
void displayMessage(String line1, String line2, String line3, String line4) {
  if (!oledOK) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 16);
  display.println(line2);
  display.setCursor(0, 32);
  display.println(line3);
  display.setCursor(0, 48);
  display.println(line4);
  
  display.display();
}

/*
 * =====================================================
 * PIN CONFIGURATION SUMMARY
 * =====================================================
 * 
 * I2C BUS (Shared by OLED + ADS1115):
 * -----------------------------------
 * | Signal | ESP32 Pin |
 * |--------|-----------|
 * | SDA    | GPIO 21   |
 * | SCL    | GPIO 22   |
 * | VCC    | 3.3V      |
 * | GND    | GND       |
 * 
 * 
 * OLED Display (0.96" SSD1306):
 * ----------------------------
 * | OLED Pin | ESP32 Pin |
 * |----------|-----------|
 * | GND      | GND       |
 * | VDD      | 3.3V      |
 * | SDA      | GPIO 21   |
 * | SCK/SCL  | GPIO 22   |
 * 
 * 
 * ADS1115 (16-bit ADC):
 * --------------------
 * | ADS1115 Pin | ESP32 Pin |
 * |-------------|-----------|
 * | VDD         | 3.3V      |
 * | GND         | GND       |
 * | SDA         | GPIO 21   |
 * | SCL         | GPIO 22   |
 * | ADDR        | GND (for 0x48) |
 * 
 * 
 * Sensors (connected to ADS1115):
 * -------------------------------
 * | Sensor          | ADS1115 Pin |
 * |-----------------|-------------|
 * | Soil Sensor AO  | A0          |
 * | Voltage Sensor  | A1          |
 * | Sensor VCC      | 3.3V        |
 * | Sensor GND      | GND         |
 * 
 * 
 * Alert Devices:
 * --------------
 * | Device  | ESP32 Pin |
 * |---------|-----------|
 * | Buzzer+ | GPIO 27   |
 * | Buzzer- | GND       |
 * | LED+    | GPIO 26 (via 220Ω resistor) |
 * | LED-    | GND       |
 * 
 * =====================================================
 * CALIBRATION INSTRUCTIONS
 * =====================================================
 * 
 * 1. Upload this code and open Serial Monitor (115200 baud)
 * 2. Check if I2C devices are found (OLED = 0x3C, ADS1115 = 0x48)
 * 3. For soil sensor calibration:
 *    - Put sensor in DRY soil → note "Soil Raw" value
 *    - Put sensor in WET soil → note "Soil Raw" value
 *    - Update SOIL_DRY_RAW and SOIL_WET_RAW in code
 * 4. For voltage sensor calibration:
 *    - Disconnect voltage → note "Voltage Raw" value
 *    - Connect voltage → note "Voltage Raw" value
 *    - Update VOLTAGE_THRESHOLD (should be between the two values)
 * 
 * =====================================================
 * TROUBLESHOOTING
 * =====================================================
 * 
 * Problem: "No I2C devices found"
 * Solution: Check wiring - SDA→21, SCL→22, VCC→3.3V, GND→GND
 * 
 * Problem: OLED not working
 * Solution: Try changing OLED_ADDRESS from 0x3C to 0x3D
 * 
 * Problem: Moisture always 0% or 100%
 * Solution: Calibrate SOIL_DRY_RAW and SOIL_WET_RAW values
 * 
 * Problem: Alert always ON
 * Solution: Increase SOIL_ALERT_PERCENT or check sensor connection
 * 
 */
