#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <WiFi.h>
#include <WebServer.h>

// ===== WiFi Configuration =====
const char* ssid = "Earthing_System";
const char* password = "12345678";

// ===== OLED Configuration =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Pin Definitions =====
#define SOIL_POWER 25
#define SOIL_PIN   34
#define ZMPT_PIN   35
#define BUZZER_PIN 26
#define RED_LED    14

// ===== Calibration =====
#define VOLTAGE_CAL 0.230  

// ===== Web Server =====
WebServer server(80);

// ===== Global Variables =====
volatile int moisturePercent = 0;
volatile float acVoltage = 0.0;
volatile bool earthingGood = false;

// ===== Task Handles =====
TaskHandle_t SensorTask;
TaskHandle_t WebServerTask;

// ===== ULTRA-SIMPLE HTML (Version 3.0) =====
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Earthing v3</title>
<style>
body{margin:0;padding:20px;background:#0a0e27;color:#fff;font-family:Arial,sans-serif}
.box{background:#1a1f3a;border-radius:10px;padding:20px;margin:10px 0;border:2px solid #2a3f5f}
h1{text-align:center;color:#4a9eff;margin:0 0 20px 0;font-size:24px}
.big{font-size:48px;font-weight:bold;text-align:center;margin:10px 0}
.label{font-size:14px;color:#8899aa;text-transform:uppercase;margin-bottom:5px}
.good{color:#00ff88}
.bad{color:#ff4444}
.bar{height:20px;background:#2a3f5f;border-radius:10px;overflow:hidden;margin:10px 0}
.fill{height:100%;background:#4a9eff;transition:width 0.3s}
.status{text-align:center;padding:30px;font-size:32px;font-weight:bold}
.conn{text-align:center;padding:5px;font-size:12px;background:#2a3f5f;border-radius:5px;margin-bottom:10px}
.info{font-size:11px;line-height:1.8;color:#8899aa}
</style>
</head><body>
<div class="conn" id="c">⏳ CONNECTING...</div>
<h1>⚡ EARTHING MONITOR v3.0</h1>
<div class="box status" id="s">CHECKING...</div>
<div class="box">
<div class="label">💧 SOIL MOISTURE</div>
<div class="big" id="sm">--</div>
<div class="bar"><div class="fill" id="sb"></div></div>
<div class="label">Target: ≥25%</div>
</div>
<div class="box">
<div class="label">⚡ AC VOLTAGE</div>
<div class="big" id="v">--</div>
<div class="bar"><div class="fill" id="vb"></div></div>
<div class="label">Range: 2.50-2.75V</div>
</div>
<div class="box">
<div class="label">📍 SYSTEM</div>
<div class="info">
Soil: GPIO 34/25 | ZMPT: GPIO 35<br>
Buzzer: GPIO 26 | LED: GPIO 14 | Cal: 0.230
</div>
</div>
<script>
setInterval(function(){
fetch('/data').then(r=>r.json()).then(d=>{
document.getElementById('sm').innerHTML=d.moisture+'%';
document.getElementById('v').innerHTML=d.voltage.toFixed(2)+'V';
document.getElementById('sb').style.width=d.moisture+'%';
document.getElementById('vb').style.width=Math.min(100,(d.voltage-2)*50)+'%';
var sg=d.moisture>=25;
var vg=d.voltage>=2.5&&d.voltage<=2.75;
document.getElementById('sm').className='big '+(sg?'good':'bad');
document.getElementById('v').className='big '+(vg?'good':'bad');
if(d.earthingGood){
document.getElementById('s').innerHTML='✓ EARTHING GOOD';
document.getElementById('s').className='box status good';
}else{
document.getElementById('s').innerHTML='⚠ EARTHING BAD';
document.getElementById('s').className='box status bad';
}
document.getElementById('c').innerHTML='✓ CONNECTED';
document.getElementById('c').style.background='#00ff88';
document.getElementById('c').style.color='#000';
}).catch(e=>{
document.getElementById('c').innerHTML='✗ ERROR: '+e.message;
document.getElementById('c').style.background='#ff4444';
console.error('Fetch failed:',e);
});
},1000);
</script>
</body></html>
)rawliteral";

// ===== Web Server Handlers =====
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleData() {
  String json = "{\"moisture\":" + String(moisturePercent) + 
                ",\"voltage\":" + String(acVoltage, 2) + 
                ",\"earthingGood\":" + String(earthingGood ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

// ===== Sensor Functions =====
float readACVoltage() {
  const int samples = 500;
  float sumSquares = 0;

  for (int i = 0; i < samples; i++) {
    int adc = analogRead(ZMPT_PIN);
    float voltage = (adc - 2048) * (3.3 / 4095.0);
    sumSquares += voltage * voltage;
    delayMicroseconds(200);
  }

  float vrms = sqrt(sumSquares / samples);
  return vrms * VOLTAGE_CAL * 100;
}

void beepAlert() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  delay(200);
}

void updateOLED() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("EARTHING MONITOR");
  display.println("----------------");
  display.print("Soil: ");
  display.print(moisturePercent);
  display.println(" %");
  display.print("Volt: ");
  display.print(acVoltage, 2);
  display.println(" V");
  display.println();
  if (earthingGood) {
    display.println("STATUS: GOOD");
  } else {
    display.println("ALERT: BAD!");
  }
  display.setCursor(0, 56);
  display.print("IP:");
  display.print(WiFi.softAPIP());
  display.display();
}

// ===== TASK 1: Web Server (Core 0) =====
void WebServerTaskCode(void * parameter) {
  Serial.println("Web Server Task started on Core 0");
  for(;;) {
    server.handleClient();
    vTaskDelay(1);
  }
}

// ===== TASK 2: Sensors (Core 1) =====
void SensorTaskCode(void * parameter) {
  Serial.println("Sensor Task started on Core 1");
  for(;;) {
    // Soil Moisture
    digitalWrite(SOIL_POWER, HIGH);
    delay(10);
    int soilValue = analogRead(SOIL_PIN);
    digitalWrite(SOIL_POWER, LOW);
    moisturePercent = map(soilValue, 4095, 0, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    // AC Voltage
    acVoltage = readACVoltage();

    // Earthing Logic
    earthingGood = false;
    if (moisturePercent >= 25 && acVoltage >= 2.50 && acVoltage <= 2.75) {
      earthingGood = true;
    }

    // Buzzer/LED Control
    if (!earthingGood) {
      digitalWrite(RED_LED, HIGH);
      beepAlert();
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }

    // Update OLED
    updateOLED();

    // Debug
    Serial.print("Soil: ");
    Serial.print(moisturePercent);
    Serial.print("% | Voltage: ");
    Serial.print(acVoltage, 2);
    Serial.print("V | Status: ");
    Serial.println(earthingGood ? "GOOD" : "BAD");
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Serial.println("\n=============================");
  Serial.println("ESP32 Earthing Monitor v3.0");
  Serial.println("ULTRA-SIMPLE DASHBOARD");
  Serial.println("=============================");

  // Pin Setup
  pinMode(SOIL_POWER, OUTPUT);
  digitalWrite(SOIL_POWER, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED, LOW);
  analogSetAttenuation(ADC_11db);

  // OLED Setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Starting AP...");
  display.display();

  // WiFi AP Setup
  WiFi.mode(WIFI_AP);
  delay(100);
  
  bool apSuccess = WiFi.softAP(ssid, password);
  
  if (apSuccess) {
    Serial.println("AP Started!");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi AP Ready!");
    display.println();
    display.print("SSID: ");
    display.println(ssid);
    display.print("Pass: ");
    display.println(password);
    display.println();
    display.print("IP: ");
    display.println(WiFi.softAPIP());
    display.display();
    delay(2000);
  } else {
    Serial.println("AP Failed!");
  }

  // Web Server Routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web server started!");
  Serial.println("Dashboard: http://192.168.4.1");

  // Create FreeRTOS Tasks
  xTaskCreatePinnedToCore(WebServerTaskCode, "WebServer", 10000, NULL, 1, &WebServerTask, 0);
  xTaskCreatePinnedToCore(SensorTaskCode, "Sensors", 10000, NULL, 1, &SensorTask, 1);
}

// ===== LOOP =====
void loop() {
  vTaskDelay(1000);
}
