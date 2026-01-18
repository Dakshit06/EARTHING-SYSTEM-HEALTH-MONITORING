/*
 * TEST 4: Buzzer & LED Test
 * Tests GPIO outputs for buzzer and LED
 * 
 * Pins:
 *   GPIO 27 = Buzzer
 *   GPIO 26 = Red LED (via 220Ω resistor)
 */

#define BUZZER_PIN 27
#define LED_PIN 26

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("\n=============================");
  Serial.println("   BUZZER & LED TEST");
  Serial.println("=============================\n");
  Serial.println("Starting test sequence...\n");
}

void loop() {
  // Test 1: LED only
  Serial.println("LED ON (3 sec)");
  digitalWrite(LED_PIN, HIGH);
  delay(3000);
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED OFF\n");
  delay(1000);
  
  // Test 2: Buzzer only
  Serial.println("BUZZER ON (1 sec)");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("BUZZER OFF\n");
  delay(1000);
  
  // Test 3: Both together (alert mode)
  Serial.println("ALERT MODE (LED + Buzzer)");
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
  Serial.println("Alert complete\n");
  
  // Test 4: LED blink pattern
  Serial.println("LED Blink (5x)");
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
  
  Serial.println("\n--- Cycle Complete ---\n");
  delay(3000);
}
