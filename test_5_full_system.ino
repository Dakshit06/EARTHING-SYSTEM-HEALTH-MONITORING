/*
 * TEST 5: Full System Test
 * Tests ALL components together with calibration mode
 * 
 * Shows raw values for calibration and allows testing alerts
 */

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ADS1115
Adafruit_ADS1115 ads;

// Pins
#define BUZZER_PIN 27
#define LED_PIN 26

// Status
bool oledOK = false;
bool adsOK = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  Wire.begin(21, 22);
  
  Serial.println("\n=============================");
  Serial.println("   FULL SYSTEM TEST");
  Serial.println("=============================\n");
  
  // Test OLED
  Serial.print("OLED: ");
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledOK = true;
    Serial.println("OK");
  } else {
    Serial.println("FAILED");
  }
  
  // Test ADS1115
  Serial.print("ADS1115: ");
  if (ads.begin(0x48)) {
    adsOK = true;
    ads.setGain(GAIN_ONE);
    Serial.println("OK");
  } else {
    Serial.println("FAILED");
  }
  
  // Test LED
  Serial.print("LED: ");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  Serial.println("TESTED (should have blinked)");
  
  // Test Buzzer
  Serial.print("Buzzer: ");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("TESTED (should have beeped)");
  
  Serial.println("\n--- Starting Monitoring ---\n");
  delay(1000);
}

void loop() {
  // Read sensors
  int16_t soilRaw = 0;
  int16_t voltRaw = 0;
  
  if (adsOK) {
    soilRaw = ads.readADC_SingleEnded(0);
    voltRaw = ads.readADC_SingleEnded(1);
  }
  
  // Calculate percentage (using default calibration)
  int soilPercent = map(soilRaw, 17000, 6000, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);
  
  // Display on OLED
  if (oledOK) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(0, 0);
    display.println("== CALIBRATION MODE ==");
    
    display.setCursor(0, 15);
    display.print("Soil Raw: ");
    display.println(soilRaw);
    
    display.setCursor(0, 27);
    display.print("Soil %: ");
    display.print(soilPercent);
    display.println("%");
    
    display.setCursor(0, 39);
    display.print("Volt Raw: ");
    display.println(voltRaw);
    
    display.setCursor(0, 55);
    if (soilPercent <= 30) {
      display.println("STATUS: ALERT!");
    } else {
      display.println("STATUS: OK");
    }
    
    display.display();
  }
  
  // Serial output
  Serial.print("Soil Raw: ");
  Serial.print(soilRaw);
  Serial.print(" | %: ");
  Serial.print(soilPercent);
  Serial.print(" | Volt Raw: ");
  Serial.print(voltRaw);
  
  // Test alert if moisture low
  if (soilPercent <= 30) {
    Serial.println(" | ALERT!");
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    Serial.println(" | OK");
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
  
  delay(1000);
}
