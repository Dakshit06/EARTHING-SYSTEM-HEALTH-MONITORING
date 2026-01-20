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

// ===== Global Variables (volatile for multi-core safety) =====
volatile int moisturePercent = 0;
volatile float acVoltage = 0.0;
volatile bool earthingGood = false;

// ===== Task Handles =====
TaskHandle_t SensorTask;
TaskHandle_t WebServerTask;

// ===== OFFLINE HTML (No External Dependencies) =====
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Earthing Monitor</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #1e3a8a 0%, #0f172a 100%);
            color: #fff;
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1000px;
            margin: 0 auto;
        }
        .header {
            text-align: center;
            padding: 20px;
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            margin-bottom: 30px;
            backdrop-filter: blur(10px);
        }
        .header h1 {
            font-size: 2rem;
            margin-bottom: 5px;
        }
        .status-badge {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.9rem;
            margin-top: 10px;
        }
        .status-badge.connected {
            background: #10b981;
        }
        .status-badge.disconnected {
            background: #ef4444;
        }
        .main-status {
            background: rgba(255,255,255,0.1);
            border-radius: 20px;
            padding: 40px;
            text-align: center;
            margin-bottom: 30px;
            backdrop-filter: blur(10px);
        }
        .status-circle {
            width: 150px;
            height: 150px;
            border-radius: 50%;
            margin: 0 auto 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 4rem;
            border: 8px solid;
            transition: all 0.3s;
        }
        .status-circle.good {
            border-color: #10b981;
            color: #10b981;
            box-shadow: 0 0 30px rgba(16,185,129,0.5);
        }
        .status-circle.bad {
            border-color: #ef4444;
            color: #ef4444;
            box-shadow: 0 0 30px rgba(239,68,68,0.5);
            animation: pulse 2s infinite;
        }
        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.05); }
        }
        .status-text {
            font-size: 2rem;
            font-weight: bold;
            margin-top: 10px;
        }
        .status-text.good { color: #10b981; }
        .status-text.bad { 
            color: #ef4444;
            animation: blink 1s infinite;
        }
        @keyframes blink {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        .metrics {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .metric-card {
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
        }
        .metric-card h3 {
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 1px;
            color: #94a3b8;
            margin-bottom: 15px;
        }
        .metric-value {
            font-size: 3rem;
            font-weight: bold;
            margin-bottom: 10px;
        }
        .metric-value.good { color: #10b981; }
        .metric-value.warning { color: #f59e0b; }
        .metric-value.bad { color: #ef4444; }
        .progress-bar {
            width: 100%;
            height: 10px;
            background: rgba(255,255,255,0.2);
            border-radius: 5px;
            overflow: hidden;
            margin-bottom: 10px;
        }
        .progress-fill {
            height: 100%;
            transition: width 0.5s, background-color 0.3s;
        }
        .metric-footer {
            font-size: 0.85rem;
            color: #94a3b8;
        }
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
        }
        .info-item {
            text-align: center;
        }
        .info-label {
            font-size: 0.75rem;
            color: #94a3b8;
            text-transform: uppercase;
            margin-bottom: 5px;
        }
        .info-value {
            font-size: 1.1rem;
            font-weight: bold;
            color: #3b82f6;
        }
        .footer {
            text-align: center;
            margin-top: 30px;
            color: #94a3b8;
            font-size: 0.9rem;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>⚡ EARTHING MONITOR</h1>
            <p>ESP32 Real-time Safety System</p>
            <span class="status-badge disconnected" id="conn-badge">CONNECTING...</span>
        </div>

        <div class="main-status">
            <div class="status-circle good" id="status-circle">✓</div>
            <div class="status-text good" id="status-text">EARTHING GOOD</div>
        </div>

        <div class="metrics">
            <div class="metric-card">
                <h3>💧 Soil Moisture</h3>
                <div class="metric-value good" id="soil-value">--</div>
                <div class="progress-bar">
                    <div class="progress-fill" id="soil-bar" style="width: 0%; background: #10b981;"></div>
                </div>
                <div class="metric-footer">Target: ≥25% for good earthing</div>
            </div>

            <div class="metric-card">
                <h3>⚡ AC Voltage (ZMPT101B)</h3>
                <div class="metric-value good" id="voltage-value">--</div>
                <div class="progress-bar">
                    <div class="progress-fill" id="voltage-bar" style="width: 0%; background: #10b981;"></div>
                </div>
                <div class="metric-footer">Safe Range: 2.50V - 2.75V</div>
            </div>
        </div>

        <div class="info-grid">
            <div class="info-item">
                <div class="info-label">Soil Sensor</div>
                <div class="info-value">GPIO 34</div>
            </div>
            <div class="info-item">
                <div class="info-label">Soil Power</div>
                <div class="info-value">GPIO 25</div>
            </div>
            <div class="info-item">
                <div class="info-label">ZMPT101B</div>
                <div class="info-value">GPIO 35</div>
            </div>
            <div class="info-item">
                <div class="info-label">Buzzer</div>
                <div class="info-value">GPIO 26</div>
            </div>
            <div class="info-item">
                <div class="info-label">Red LED</div>
                <div class="info-value">GPIO 14</div>
            </div>
            <div class="info-item">
                <div class="info-label">Calibration</div>
                <div class="info-value">0.230</div>
            </div>
        </div>

        <div class="footer">
            <p>ESP32 Earthing Monitor • <span id="clock">00:00:00</span></p>
        </div>
    </div>

    <script>
        const soilValue = document.getElementById('soil-value');
        const voltageValue = document.getElementById('voltage-value');
        const soilBar = document.getElementById('soil-bar');
        const voltageBar = document.getElementById('voltage-bar');
        const statusCircle = document.getElementById('status-circle');
        const statusText = document.getElementById('status-text');
        const connBadge = document.getElementById('conn-badge');
        const clockEl = document.getElementById('clock');

        function updateClock() {
            clockEl.textContent = new Date().toLocaleTimeString();
        }

        async function fetchData() {
            try {
                const response = await fetch('/data');
                if (!response.ok) throw new Error('Network error');
                const data = await response.json();
                
                // Update values
                soilValue.textContent = data.moisture + '%';
                voltageValue.textContent = data.voltage.toFixed(2) + 'V';
                
                // Update soil bar
                soilBar.style.width = data.moisture + '%';
                if (data.moisture >= 25) {
                    soilValue.className = 'metric-value good';
                    soilBar.style.background = '#10b981';
                } else {
                    soilValue.className = 'metric-value warning';
                    soilBar.style.background = '#f59e0b';
                }
                
                // Update voltage bar
                const voltPercent = Math.min(100, Math.max(0, ((data.voltage - 2.0) / 1.0) * 100));
                voltageBar.style.width = voltPercent + '%';
                if (data.voltage >= 2.50 && data.voltage <= 2.75) {
                    voltageValue.className = 'metric-value good';
                    voltageBar.style.background = '#10b981';
                } else {
                    voltageValue.className = 'metric-value bad';
                    voltageBar.style.background = '#ef4444';
                }
                
                // Update main status
                if (data.earthingGood) {
                    statusCircle.className = 'status-circle good';
                    statusCircle.textContent = '✓';
                    statusText.className = 'status-text good';
                    statusText.textContent = 'EARTHING GOOD';
                } else {
                    statusCircle.className = 'status-circle bad';
                    statusCircle.textContent = '⚠';
                    statusText.className = 'status-text bad';
                    statusText.textContent = 'EARTHING BAD';
                }
                
                // Update connection badge
                connBadge.textContent = 'CONNECTED';
                connBadge.className = 'status-badge connected';
                
            } catch (error) {
                console.error('Fetch error:', error);
                connBadge.textContent = 'DISCONNECTED';
                connBadge.className = 'status-badge disconnected';
            }
        }

        setInterval(updateClock, 1000);
        setInterval(fetchData, 1000);
        updateClock();
        fetchData();
    </script>
</body>
</html>
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

// ===== YOUR ORIGINAL FUNCTIONS =====
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
    display.println("STATUS:");
    display.println("EARTHING GOOD");
  } else {
    display.println("ALERT!");
    display.println("EARTHING BAD");
  }

  display.setCursor(0, 56);
  display.print("IP:");
  display.print(WiFi.softAPIP());

  display.display();
}

// ===== TASK 1: Web Server (runs on Core 0) =====
void WebServerTaskCode(void * parameter) {
  Serial.println("Web Server Task started on Core 0");
  
  for(;;) {
    server.handleClient();
    vTaskDelay(1);
  }
}

// ===== TASK 2: Sensor Reading (runs on Core 1) =====
void SensorTaskCode(void * parameter) {
  Serial.println("Sensor Task started on Core 1");
  
  for(;;) {
    // ===== Soil Moisture =====
    digitalWrite(SOIL_POWER, HIGH);
    delay(10);
    int soilValue = analogRead(SOIL_PIN);
    digitalWrite(SOIL_POWER, LOW);

    moisturePercent = map(soilValue, 4095, 0, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    // ===== ZMPT Voltage =====
    acVoltage = readACVoltage();

    // ===== EARTHING LOGIC =====
    earthingGood = false;
    if (moisturePercent >= 25 &&
        acVoltage >= 2.50 &&
        acVoltage <= 2.75) {
      earthingGood = true;
    }

    // ===== RED LED + BUZZER CONTROL =====
    if (!earthingGood) {
      digitalWrite(RED_LED, HIGH);
      beepAlert();
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }

    // ===== UPDATE OLED =====
    updateOLED();

    // Debug output
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
  Serial.println("ESP32 Earthing Monitor");
  Serial.println("OFFLINE DUAL-CORE VERSION");
  Serial.println("=============================");

  // Pin Setup
  pinMode(SOIL_POWER, OUTPUT);
  digitalWrite(SOIL_POWER, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED, LOW);

  // ADC Setup
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

  // ===== WiFi Access Point Setup =====
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

  // ===== Create FreeRTOS Tasks on Different Cores =====
  
  // Task 1: Web Server on Core 0
  xTaskCreatePinnedToCore(
    WebServerTaskCode,
    "WebServerTask",
    10000,
    NULL,
    1,
    &WebServerTask,
    0
  );

  // Task 2: Sensors on Core 1
  xTaskCreatePinnedToCore(
    SensorTaskCode,
    "SensorTask",
    10000,
    NULL,
    1,
    &SensorTask,
    1
  );
}

// ===== LOOP (empty - tasks handle everything) =====
void loop() {
  vTaskDelay(1000);
}
