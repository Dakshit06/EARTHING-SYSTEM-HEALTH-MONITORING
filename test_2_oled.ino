/*
 * TEST 2: OLED Display Test
 * Tests if OLED is working properly
 * 
 * If blank screen: Try changing address from 0x3C to 0x3D
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C  // Change to 0x3D if not working

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(21, 22);
  
  Serial.println("\n=============================");
  Serial.println("   OLED DISPLAY TEST");
  Serial.println("=============================\n");
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED FAILED!");
    Serial.println("Try address 0x3D or check wiring");
    while (1);
  }
  
  Serial.println("OLED OK!");
  
  // Clear and show test pattern
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 10);
  display.println("OLED");
  display.setCursor(30, 35);
  display.println("TEST");
  display.display();
}

void loop() {
  // Test 1: Text
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Line 1: OLED Working!");
  display.println("Line 2: Test Message");
  display.println("Line 3: ESP32 Ready");
  display.println();
  display.setTextSize(2);
  display.println("SUCCESS!");
  display.display();
  delay(2000);
  
  // Test 2: Counter
  for (int i = 5; i >= 0; i--) {
    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(50, 20);
    display.println(i);
    display.display();
    delay(1000);
  }
  
  // Test 3: Graphics
  display.clearDisplay();
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.drawLine(0, 0, 127, 63, SSD1306_WHITE);
  display.drawLine(127, 0, 0, 63, SSD1306_WHITE);
  display.display();
  delay(2000);
}
