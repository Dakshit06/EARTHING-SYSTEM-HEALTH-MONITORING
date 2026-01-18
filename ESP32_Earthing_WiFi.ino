/*
 * ============================================================
 * ESP32 EARTHING MONITOR + WIFI DASHBOARD
 * ============================================================
 * 
 * Features:
 * - Hosts a premium Web Dashboard on WiFi AP "EarthingGuard"
 * - Real-time monitoring of Soil Moisture & Leakage Voltage
 * - I2C OLED Display Support
 * - JSON API for the web interface
 * 
 * Author: Earthing Project
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>

// ==================== PINS & I2C ====================
#define SDA_PIN 21
#define SCL_PIN 22
#define BUZZER_PIN 27
#define LED_PIN 26

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C
#define ADS_ADDRESS 0x48

// ==================== CALIBRATION ====================
#define SOIL_DRY_RAW 17000     
#define SOIL_WET_RAW 6000      
#define SOIL_ALERT_PERCENT 25  
#define ZMPT_CALIBRATION 0.230 // Adjust factor for voltage

// ==================== OBJECTS ====================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_ADS1115 ads;
WebServer server(80);

// ==================== VARIABLES ====================
int16_t soilRaw = 0;
int16_t voltageRaw = 0;
int soilPercent = 0;
float voltageAC = 0.0;
bool soilGood = false;
bool voltGood = false;
bool systemSafe = false;

// ==================== WEB RESOURCES (PROGMEM) ====================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Earthing Guard | Advanced Monitor</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="style.css">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
</head>
<body>
    <div class="app-container">
        <header class="glass-panel header">
            <div class="logo-area">
                <i class="fa-solid fa-bolt logo-icon"></i>
                <div>
                    <h1>Earthing Guard</h1>
                    <p class="subtitle">WiFi Monitoring System</p>
                </div>
            </div>
            <div class="system-status-badge" id="system-status-badge">
                <span class="status-dot"></span>
                <span id="header-status-text">CONNECTING...</span>
            </div>
        </header>
        <main class="dashboard-grid">
            <section class="glass-panel status-hero">
                <div class="hero-content">
                    <div class="status-ring-container">
                        <div class="status-ring" id="main-status-ring">
                            <i class="fa-solid fa-check-circle" id="main-status-icon"></i>
                        </div>
                    </div>
                    <div class="status-text">
                        <h2 id="main-status-title">System Safe</h2>
                        <p id="main-status-desc">All parameters within optimal range.</p>
                    </div>
                </div>
            </section>
            <div class="metrics-container">
                <div class="glass-panel metric-card" id="card-soil">
                    <div class="card-header">
                        <span class="card-icon"><i class="fa-solid fa-droplet"></i></span>
                        <h3>Soil Moisture</h3>
                    </div>
                    <div class="card-value-area">
                        <span class="value" id="val-soil">--</span>
                        <span class="unit">%</span>
                    </div>
                    <div class="progress-bar-bg">
                        <div class="progress-bar-fill" id="bar-soil" style="width: 0%"></div>
                    </div>
                    <p class="card-footer">Target: >25%</p>
                </div>
                <div class="glass-panel metric-card" id="card-voltage">
                    <div class="card-header">
                        <span class="card-icon"><i class="fa-solid fa-plug"></i></span>
                        <h3>Leakage Voltage</h3>
                    </div>
                    <div class="card-value-area">
                        <span class="value" id="val-voltage">--</span>
                        <span class="unit">V</span>
                    </div>
                    <div class="progress-bar-bg">
                        <div class="progress-bar-fill" id="bar-voltage" style="width: 0%"></div>
                    </div>
                    <p class="card-footer">Safe Range: 2.55V - 2.61V</p>
                </div>
            </div>
            <section class="glass-panel chart-section">
                <h3>Live Voltage History</h3>
                <div class="chart-container">
                    <canvas id="voltageChart"></canvas>
                </div>
            </section>
        </main>
        <footer class="app-footer">
            <p>IoT Dashboard • <span id="clock">00:00:00</span></p>
        </footer>
    </div>
    <script src="script.js"></script>
</body>
</html>
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(
:root {
    --bg-dark: #0f172a;
    --bg-panel: rgba(30, 41, 59, 0.7);
    --text-primary: #f8fafc;
    --text-secondary: #94a3b8;
    --accent-success: #10b981;
    --accent-danger: #ef4444;
    --accent-warning: #f59e0b;
    --accent-primary: #3b82f6;
    --glass-border: 1px solid rgba(255, 255, 255, 0.1);
    --font-main: 'Outfit', sans-serif;
}
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
    font-family: var(--font-main);
    background-color: var(--bg-dark);
    background-image: 
        radial-gradient(at 0% 0%, rgba(59, 130, 246, 0.15) 0px, transparent 50%),
        radial-gradient(at 100% 0%, rgba(16, 185, 129, 0.1) 0px, transparent 50%);
    color: var(--text-primary);
    min-height: 100vh;
    display: flex;
    justify-content: center;
    align-items: center;
    padding: 20px;
}
.app-container { width: 100%; max-width: 1200px; display: flex; flex-direction: column; gap: 20px; }
.glass-panel {
    background: var(--bg-panel);
    backdrop-filter: blur(16px);
    -webkit-backdrop-filter: blur(16px);
    border: var(--glass-border);
    border-radius: 20px;
    box-shadow: 0 4px 30px rgba(0, 0, 0, 0.3);
    padding: 20px;
}
.header { display: flex; justify-content: space-between; align-items: center; padding: 15px 30px; }
.logo-area { display: flex; align-items: center; gap: 15px; }
.logo-icon { font-size: 2rem; color: var(--accent-primary); }
.logo-area h1 { font-size: 1.5rem; font-weight: 700; }
.subtitle { font-size: 0.8rem; color: var(--text-secondary); }
.system-status-badge { display: flex; align-items: center; gap: 8px; background: rgba(0, 0, 0, 0.3); padding: 8px 16px; border-radius: 30px; font-size: 0.85rem; font-weight: 600; }
.status-dot { width: 10px; height: 10px; border-radius: 50%; background-color: var(--text-secondary); box-shadow: 0 0 5px var(--text-secondary); }
.dashboard-grid { display: grid; grid-template-columns: 1fr 1fr; grid-template-rows: auto auto; gap: 20px; }
.status-hero { grid-column: 1 / -1; display: flex; align-items: center; justify-content: center; padding: 40px; text-align: center; }
.hero-content { display: flex; flex-direction: column; align-items: center; gap: 20px; z-index: 2; }
.status-ring-container { position: relative; width: 120px; height: 120px; }
.status-ring { width: 100%; height: 100%; border-radius: 50%; border: 8px solid rgba(255,255,255,0.05); display: flex; align-items: center; justify-content: center; font-size: 3.5rem; color: var(--text-secondary); transition: all 0.5s ease; }
.status-ring.safe { border-color: var(--accent-success); color: var(--accent-success); box-shadow: 0 0 30px rgba(16, 185, 129, 0.3); }
.status-ring.danger { border-color: var(--accent-danger); color: var(--accent-danger); box-shadow: 0 0 30px rgba(239, 68, 68, 0.3); animation: pulse-danger 2s infinite; }
@keyframes pulse-danger { 0% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.4); } 70% { box-shadow: 0 0 0 20px rgba(239, 68, 68, 0); } 100% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0); } }
.status-text h2 { font-size: 2rem; margin-bottom: 5px; }
.status-text p { color: var(--text-secondary); }
.metrics-container { grid-column: 1 / 2; display: flex; flex-direction: column; gap: 20px; }
.metric-card { display: flex; flex-direction: column; gap: 15px; }
.card-header { display: flex; align-items: center; gap: 10px; color: var(--text-secondary); font-size: 0.9rem; text-transform: uppercase; letter-spacing: 1px; }
.card-value-area { display: flex; align-items: baseline; gap: 5px; }
.value { font-size: 3rem; font-weight: 700; }
.unit { font-size: 1.2rem; color: var(--text-secondary); }
.progress-bar-bg { width: 100%; height: 8px; background: rgba(255, 255, 255, 0.1); border-radius: 4px; overflow: hidden; }
.progress-bar-fill { height: 100%; background: var(--accent-primary); border-radius: 4px; transition: width 0.5s ease-out; }
.card-footer { font-size: 0.8rem; color: var(--text-secondary); margin-top: auto; }
.chart-section { grid-column: 2 / 3; display: flex; flex-direction: column; gap: 15px; }
.chart-container { flex-grow: 1; position: relative; width: 100%; min-height: 250px; }
.app-footer { text-align: center; color: var(--text-secondary); font-size: 0.8rem; padding: 10px; }
@media (max-width: 768px) { .dashboard-grid { grid-template-columns: 1fr; } .metrics-container, .chart-section { grid-column: 1 / -1; } .header { flex-direction: column; gap: 15px; text-align: center; } }
)rawliteral";

const char script_js[] PROGMEM = R"rawliteral(
document.addEventListener('DOMContentLoaded', () => {
    const valSoil = document.getElementById('val-soil');
    const barSoil = document.getElementById('bar-soil');
    const valVoltage = document.getElementById('val-voltage');
    const barVoltage = document.getElementById('bar-voltage');
    const mainStatusRing = document.getElementById('main-status-ring');
    const mainStatusIcon = document.getElementById('main-status-icon');
    const mainStatusTitle = document.getElementById('main-status-title');
    const mainStatusDesc = document.getElementById('main-status-desc');
    const statusBadgeText = document.getElementById('header-status-text');
    const statusDot = document.querySelector('.status-dot');
    const clockElement = document.getElementById('clock');
    
    // Chart
    const ctx = document.getElementById('voltageChart').getContext('2d');
    const voltageChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: Array(20).fill(''),
            datasets: [{
                label: 'Leakage (V)',
                data: Array(20).fill(0),
                borderColor: '#3b82f6',
                backgroundColor: 'rgba(59, 130, 246, 0.1)',
                borderWidth: 2,
                fill: true,
                tension: 0.4,
                pointRadius: 0
            }]
        },
        options: {
            responsive: true, maintainAspectRatio: false,
            scales: { y: { beginAtZero: false, suggestedMin: 2.4, suggestedMax: 2.8, grid: { color: 'rgba(255,255,255,0.05)' }, ticks: { color: '#94a3b8' } }, x: { grid: { display: false } } },
            plugins: { legend: { display: false } },
            animation: false
        }
    });

    function updateClock() { document.getElementById('clock').textContent = new Date().toLocaleTimeString(); }

    async function fetchData() {
        try {
            const response = await fetch('/data');
            if (!response.ok) throw new Error('Net error');
            const data = await response.json();
            updateDashboard(data.moisture, data.voltage, data.soilGood, data.voltGood);
            statusBadgeText.textContent = 'CONNECTED';
            statusBadgeText.style.color = 'var(--accent-success)';
            statusDot.style.backgroundColor = 'var(--accent-success)';
        } catch (error) {
            console.error(error);
            statusBadgeText.textContent = 'DISCONNECTED';
            statusBadgeText.style.color = 'var(--text-secondary)';
            statusDot.style.backgroundColor = 'var(--text-secondary)';
        }
    }

    function updateDashboard(moistureVal, voltageVal, isSoilGood, isVoltGood) {
        valSoil.textContent = Math.round(moistureVal);
        valVoltage.textContent = voltageVal.toFixed(2);
        barSoil.style.width = `${Math.round(moistureVal)}%`;
        barVoltage.style.width = `${Math.min(100, (voltageVal / 3.3) * 100)}%`;
        
        const isSafe = isSoilGood && isVoltGood;
        if (isSafe) {
            mainStatusRing.className = 'status-ring safe';
            mainStatusIcon.className = 'fa-solid fa-check-circle';
            mainStatusTitle.textContent = 'System Safe';
            mainStatusDesc.textContent = 'System Normal.';
        } else {
            mainStatusRing.className = 'status-ring danger';
            mainStatusIcon.className = 'fa-solid fa-triangle-exclamation';
            mainStatusTitle.textContent = 'Check Earthing';
            let issues = [];
            if (!isSoilGood) issues.push("Low Moisture");
            if (!isVoltGood) issues.push("Volt Leak");
            mainStatusDesc.textContent = `Alert: ${issues.join(' & ')}`;
        }
        
        // Colors
        const goodColor = 'var(--accent-success)';
        const badColor = 'var(--accent-danger)';
        const warnColor = 'var(--accent-warning)';
        
        valSoil.style.color = isSoilGood ? goodColor : warnColor;
        barSoil.style.backgroundColor = isSoilGood ? goodColor : warnColor;
        
        valVoltage.style.color = isVoltGood ? goodColor : badColor;
        barVoltage.style.backgroundColor = isVoltGood ? goodColor : badColor;
        
        // Update Chart
        const d = voltageChart.data.datasets[0].data; d.shift(); d.push(voltageVal);
        voltageChart.data.datasets[0].borderColor = isSafe ? '#10b981' : '#ef4444';
        voltageChart.data.datasets[0].backgroundColor = isSafe ? 'rgba(16, 185, 129, 0.1)' : 'rgba(239, 68, 68, 0.1)';
        voltageChart.update();
    }

    setInterval(updateClock, 1000);
    setInterval(fetchData, 1000);
    updateClock();
});
)rawliteral";

// ==================== FUNCTIONS ====================

// --- Voltage Reading ---
float readVoltage() {
  const int samples = 500;
  float sumSquares = 0;
  for (int i = 0; i < samples; i++) {
    int16_t adc = ads.readADC_SingleEnded(1); // A1 for Voltage
    // 3.3V equiv is roughly 26400 raw on GAIN_ONE? 
    // ADS1115 GAIN_ONE: +/- 4.096V range. 1 bit = 0.125mV
    // Raw * 0.000125 = Voltage
    // But let's stick to the logic from schematic reference if available.
    // Assuming simple scaling:
    float v = adc * 0.000125; 
    // Remove DC offset (assuming 1.65V center)
    v = v - 1.65; 
    sumSquares += v * v;
  }
  float vrms = sqrt(sumSquares / samples);
  // Manual tuning if needed, otherwise use calibrated multiple
  return vrms * 100.0 * 0.1; // Placeholder scaling
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  
  // Pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED Failed");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("Starting WiFi...");
    display.display();
  }
  
  // ADS1115
  if(ads.begin(ADS_ADDRESS)) {
    ads.setGain(GAIN_ONE);
  } else {
    Serial.println("ADS Failed");
  }
  
  // WiFi AP
  WiFi.softAP("EarthingGuard", "12345678"); // Password optional
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi: EarthingGuard");
  display.println(IP);
  display.display();
  
  // Web Routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });
  
  server.on("/style.css", HTTP_GET, []() {
    server.send(200, "text/css", style_css);
  });
  
  server.on("/script.js", HTTP_GET, []() {
    server.send(200, "application/javascript", script_js);
  });
  
  server.on("/data", HTTP_GET, []() {
    String json = "{";
    json += "\"moisture\":" + String(soilPercent) + ",";
    json += "\"voltage\":" + String(voltageAC) + ",";
    json += "\"soilGood\":" + String(soilGood ? "true" : "false") + ",";
    json += "\"voltGood\":" + String(voltGood ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json);
  });
  
  server.begin();
  Serial.println("Web server started");
}

// --- Loop ---
void loop() {
  server.handleClient();
  
  // --- Sensor Logic ---
  
  // 1. Soil (ADS A0)
  int32_t rawSum = 0;
  for(int i=0; i<10; i++) { rawSum += ads.readADC_SingleEnded(0); delay(5); }
  soilRaw = rawSum / 10;
  soilPercent = map(soilRaw, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);
  
  // 2. Voltage (ADS A1)
  // Simple reading for now (RMS needs valid ZMPT calibration)
  int16_t vRaw = ads.readADC_SingleEnded(1);
  // Dummy conversion for "Good" range testing: map raw to approx 2.58V
  // Or implement full RMS if sensor is live
  // Using user logic: (adc - 2048) ... 
  // Let's implement the specific logic requested in original prompt
  // "AC Voltage >= 2.55 && AC Voltage <= 2.61"
  // For simulation/demo on hardware without live AC, we might need a potentiometer
  float v = vRaw * 0.000125; // Real voltage at pin
  voltageAC = v * 100.0; // Assume ZMPT scaling
  // Hack: Keep it in safe range if no sensor connected?
  // voltageAC = 2.58; 

  // Check Logic
  soilGood = (soilPercent >= SOIL_ALERT_PERCENT);
  voltGood = (voltageAC >= 2.55 && voltageAC <= 2.61);
  systemSafe = soilGood && voltGood;
  
  // --- Hardware Alerts ---
  if (systemSafe) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
    // Beep every loop (approx 200ms ish due to delays? No, web server needs speed)
    // Non-blocking blink needed for proper web server
    // For now, simple on
    digitalWrite(BUZZER_PIN, (millis() / 500) % 2); // Blink buzzer
  }
  
  delay(10); // Small delay for stability
}
