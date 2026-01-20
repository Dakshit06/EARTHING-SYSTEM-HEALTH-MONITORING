import http.server
import socketserver
import json
import random
import time
import webbrowser
from urllib.parse import urlparse, parse_qs

PORT = 8000

# Global simulation state
sim_mode = "AUTO" # AUTO, GOOD, BAD
forced_moisture = 40
forced_voltage = 2.58

# Extract HTML from the ESP32 code structure (simplified for python string)
# Added Simulation Controls UI
html_content = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Earthing Monitor | ESP32 System</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700&display=swap" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <style>
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
            border: var(--glass-border);
            border-radius: 20px;
            box-shadow: 0 4px 30px rgba(0, 0, 0, 0.3);
            padding: 20px;
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        .header { display: flex; justify-content: space-between; align-items: center; padding: 15px 30px; flex-wrap: wrap; gap: 15px; }
        .logo-area { display: flex; align-items: center; gap: 15px; }
        .logo-icon { font-size: 2rem; color: var(--accent-primary); text-shadow: 0 0 10px var(--accent-primary); }
        .logo-area h1 { font-size: 1.5rem; font-weight: 700; }
        .subtitle { font-size: 0.8rem; color: var(--text-secondary); }
        .system-status-badge { display: flex; align-items: center; gap: 8px; background: rgba(0,0,0,0.3); padding: 8px 16px; border-radius: 30px; font-size: 0.85rem; font-weight: 600; }
        .status-dot { width: 10px; height: 10px; border-radius: 50%; background-color: var(--text-secondary); transition: all 0.3s ease; }
        .dashboard-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }
        .status-hero { grid-column: 1 / -1; display: flex; align-items: center; justify-content: center; padding: 40px; text-align: center; }
        .hero-content { display: flex; flex-direction: column; align-items: center; gap: 20px; }
        .status-ring-container { width: 120px; height: 120px; }
        .status-ring { width: 100%; height: 100%; border-radius: 50%; border: 8px solid rgba(255,255,255,0.05); display: flex; align-items: center; justify-content: center; font-size: 3.5rem; color: var(--text-secondary); transition: all 0.5s ease; }
        .status-ring.safe { border-color: var(--accent-success); color: var(--accent-success); box-shadow: 0 0 30px rgba(16, 185, 129, 0.3); }
        .status-ring.danger { border-color: var(--accent-danger); color: var(--accent-danger); box-shadow: 0 0 30px rgba(239, 68, 68, 0.3); animation: pulse-danger 2s infinite; }
        @keyframes pulse-danger { 0% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.4); } 70% { box-shadow: 0 0 0 20px rgba(239, 68, 68, 0); } 100% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0); } }
        .status-text h2 { font-size: 2rem; margin-bottom: 5px; }
        .status-label { font-size: 1.5rem !important; font-weight: 700; letter-spacing: 2px; }
        .status-label.good { color: var(--accent-success) !important; text-shadow: 0 0 10px rgba(16, 185, 129, 0.5); }
        .status-label.bad { color: var(--accent-danger) !important; text-shadow: 0 0 10px rgba(239, 68, 68, 0.5); animation: blink-text 1s infinite; }
        @keyframes blink-text { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }
        .alert-indicators { display: flex; gap: 20px; margin-top: 15px; opacity: 0; transform: translateY(-10px); transition: all 0.3s ease; }
        .alert-indicators.active { opacity: 1; transform: translateY(0); }
        .indicator { display: flex; align-items: center; gap: 8px; padding: 8px 16px; border-radius: 20px; background: rgba(0,0,0,0.3); font-size: 0.85rem; font-weight: 600; color: var(--text-secondary); }
        .indicator.red-led.on { background: rgba(239, 68, 68, 0.2); color: var(--accent-danger); border: 1px solid var(--accent-danger); }
        .indicator.red-led.on i { animation: led-pulse 0.5s infinite; }
        @keyframes led-pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.6; } }
        .indicator.buzzer.on { background: rgba(245, 158, 11, 0.2); color: var(--accent-warning); border: 1px solid var(--accent-warning); }
        .indicator.buzzer.on i { animation: buzzer-shake 0.2s infinite; }
        @keyframes buzzer-shake { 0%, 100% { transform: translateX(0); } 25% { transform: translateX(-2px); } 75% { transform: translateX(2px); } }
        .status-hero.alert-mode { background: linear-gradient(135deg, rgba(239, 68, 68, 0.1), rgba(30, 41, 59, 0.7)); }
        .metrics-container { grid-column: 1 / 2; display: flex; flex-direction: column; gap: 20px; }
        .metric-card { display: flex; flex-direction: column; gap: 15px; }
        .metric-card.warning { border: 1px solid var(--accent-warning); box-shadow: 0 0 15px rgba(245, 158, 11, 0.2); }
        .metric-card.danger { border: 1px solid var(--accent-danger); box-shadow: 0 0 15px rgba(239, 68, 68, 0.2); }
        .card-header { display: flex; align-items: center; gap: 10px; color: var(--text-secondary); font-size: 0.9rem; text-transform: uppercase; letter-spacing: 1px; }
        .card-value-area { display: flex; align-items: baseline; gap: 5px; }
        .value { font-size: 3rem; font-weight: 700; }
        .unit { font-size: 1.2rem; color: var(--text-secondary); }
        .progress-bar-bg { width: 100%; height: 8px; background: rgba(255,255,255,0.1); border-radius: 4px; overflow: hidden; }
        .progress-bar-fill { height: 100%; background: var(--accent-primary); border-radius: 4px; transition: width 0.5s ease-out, background-color 0.3s ease; }
        .card-footer { font-size: 0.8rem; color: var(--text-secondary); margin-top: auto; }
        .chart-section { grid-column: 2 / 3; display: flex; flex-direction: column; gap: 15px; }
        .chart-container { flex-grow: 1; position: relative; width: 100%; min-height: 250px; }
        .system-info { grid-column: 1 / -1; }
        .system-info h3 { display: flex; align-items: center; gap: 10px; margin-bottom: 15px; font-size: 1rem; }
        .system-info h3 i { color: var(--accent-primary); }
        .info-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; }
        .info-item { display: flex; flex-direction: column; gap: 5px; padding: 12px; background: rgba(0,0,0,0.2); border-radius: 10px; }
        .info-label { font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; }
        .info-value { font-size: 1rem; font-weight: 600; color: var(--accent-primary); }
        .app-footer { text-align: center; color: var(--text-secondary); font-size: 0.8rem; padding: 10px; }
        
        /* Sim Controls */
        .sim-controls {
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: rgba(15, 23, 42, 0.95);
            border: 1px solid var(--accent-primary);
            padding: 15px;
            border-radius: 10px;
            z-index: 9999;
            box-shadow: 0 0 20px rgba(0,0,0,0.5);
        }
        .sim-controls h4 { margin-bottom: 10px; font-size: 0.9rem; color: var(--accent-primary); }
        .sim-btn-group { display: flex; gap: 10px; flex-wrap: wrap; }
        .sim-btn {
            padding: 8px 12px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-weight: 600;
            font-size: 0.8rem;
            transition: all 0.2s;
        }
        .btn-good { background: rgba(16, 185, 129, 0.2); color: #10b981; border: 1px solid #10b981; }
        .btn-good:hover { background: #10b981; color: #fff; }
        .btn-bad { background: rgba(239, 68, 68, 0.2); color: #ef4444; border: 1px solid #ef4444; }
        .btn-bad:hover { background: #ef4444; color: #fff; }
        .btn-auto { background: rgba(59, 130, 246, 0.2); color: #3b82f6; border: 1px solid #3b82f6; }
        .btn-auto:hover { background: #3b82f6; color: #fff; }
        
        @media (max-width: 768px) {
            .dashboard-grid { grid-template-columns: 1fr; }
            .metrics-container, .chart-section { grid-column: 1 / -1; }
            .header { flex-direction: column; text-align: center; }
        }
    </style>
</head>
<body>
    <div class="sim-controls">
        <h4>🛠 Simulation Controls</h4>
        <div class="sim-btn-group">
            <button class="sim-btn btn-good" onclick="setMode('GOOD')">Force GOOD</button>
            <button class="sim-btn btn-bad" onclick="setMode('BAD')">Force BAD</button>
            <button class="sim-btn btn-auto" onclick="setMode('AUTO')">Auto Mode</button>
        </div>
    </div>

    <div class="app-container">
        <header class="glass-panel header">
            <div class="logo-area">
                <i class="fa-solid fa-bolt logo-icon"></i>
                <div>
                    <h1>EARTHING MONITOR</h1>
                    <p class="subtitle">ESP32 Real-time Safety System (SIMULATION)</p>
                </div>
            </div>
            <div class="system-status-badge">
                <span class="status-dot"></span>
                <span id="header-status-text">INITIALIZING</span>
            </div>
        </header>
        <main class="dashboard-grid">
            <section class="glass-panel status-hero" id="status-hero">
                <div class="hero-content">
                    <div class="status-ring-container">
                        <div class="status-ring" id="main-status-ring">
                            <i class="fa-solid fa-check-circle" id="main-status-icon"></i>
                        </div>
                    </div>
                    <div class="status-text">
                        <h2 id="main-status-title">STATUS:</h2>
                        <p id="main-status-desc" class="status-label">EARTHING GOOD</p>
                    </div>
                    <div class="alert-indicators" id="alert-indicators">
                        <div class="indicator red-led" id="red-led-indicator">
                            <i class="fa-solid fa-circle"></i>
                            <span>RED LED</span>
                        </div>
                        <div class="indicator buzzer" id="buzzer-indicator">
                            <i class="fa-solid fa-volume-high"></i>
                            <span>BUZZER</span>
                        </div>
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
                    <p class="card-footer">Target: ≥25% for good earthing</p>
                </div>
                <div class="glass-panel metric-card" id="card-voltage">
                    <div class="card-header">
                        <span class="card-icon"><i class="fa-solid fa-plug"></i></span>
                        <h3>AC Voltage (ZMPT101B)</h3>
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
            <section class="glass-panel system-info">
                <h3><i class="fa-solid fa-microchip"></i> System Configuration</h3>
                <div class="info-grid">
                    <div class="info-item"><span class="info-label">Mode</span><span class="info-value">SIMULATION</span></div>
                    <div class="info-item"><span class="info-label">Host</span><span class="info-value">LOCALHOST</span></div>
                    <div class="info-item"><span class="info-label">Status</span><span class="info-value">RUNNING</span></div>
                </div>
            </section>
        </main>
        <footer class="app-footer">
            <p>ESP32 Earthing Monitor • <span id="clock">00:00:00</span></p>
        </footer>
    </div>
    <script>
        async function setMode(mode) {
            await fetch('/set_mode?mode=' + mode);
        }

        document.addEventListener('DOMContentLoaded', () => {
            const valSoil = document.getElementById('val-soil');
            const barSoil = document.getElementById('bar-soil');
            const cardSoil = document.getElementById('card-soil');
            const valVoltage = document.getElementById('val-voltage');
            const barVoltage = document.getElementById('bar-voltage');
            const cardVoltage = document.getElementById('card-voltage');
            const statusHero = document.getElementById('status-hero');
            const mainStatusRing = document.getElementById('main-status-ring');
            const mainStatusIcon = document.getElementById('main-status-icon');
            const mainStatusTitle = document.getElementById('main-status-title');
            const mainStatusDesc = document.getElementById('main-status-desc');
            const statusBadgeText = document.getElementById('header-status-text');
            const statusDot = document.querySelector('.status-dot');
            const redLedIndicator = document.getElementById('red-led-indicator');
            const buzzerIndicator = document.getElementById('buzzer-indicator');
            const alertIndicators = document.getElementById('alert-indicators');
            const clockElement = document.getElementById('clock');

            const ctx = document.getElementById('voltageChart').getContext('2d');
            const voltageChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: Array(20).fill(''),
                    datasets: [{
                        label: 'AC Voltage (V)',
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
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: { beginAtZero: false, suggestedMin: 2.4, suggestedMax: 2.8, grid: { color: 'rgba(255,255,255,0.05)' }, ticks: { color: '#94a3b8' } },
                        x: { grid: { display: false } }
                    },
                    plugins: { legend: { display: false } },
                    animation: false
                }
            });

            const SOIL_THRESHOLD = 25;
            const VOLT_MIN = 2.55;
            const VOLT_MAX = 2.61;

            function updateClock() {
                clockElement.textContent = new Date().toLocaleTimeString();
            }

            function checkEarthingStatus(moisture, voltage) {
                const soilGood = moisture >= SOIL_THRESHOLD;
                const voltGood = voltage >= VOLT_MIN && voltage <= VOLT_MAX;
                return { earthingGood: soilGood && voltGood, soilGood, voltGood };
            }

            async function fetchData() {
                try {
                    const response = await fetch('/data');
                    if (!response.ok) throw new Error('Network error');
                    const data = await response.json();
                    const { earthingGood, soilGood, voltGood } = checkEarthingStatus(data.moisture, data.voltage);
                    updateDashboard(data.moisture, data.voltage, earthingGood, soilGood, voltGood);
                    statusBadgeText.textContent = 'CONNECTED';
                    statusDot.style.backgroundColor = '#10b981';
                } catch (error) {
                    console.error('Fetch error:', error);
                    statusBadgeText.textContent = 'DISCONNECTED';
                    statusDot.style.backgroundColor = '#94a3b8';
                }
            }

            function updateDashboard(moistureVal, voltageVal, earthingGood, isSoilGood, isVoltGood) {
                const dispMoisture = Math.round(moistureVal);
                const dispVoltage = voltageVal.toFixed(2);
                valSoil.textContent = dispMoisture;
                valVoltage.textContent = dispVoltage;
                barSoil.style.width = dispMoisture + '%';
                barVoltage.style.width = Math.min(100, Math.max(0, ((voltageVal - 2.0) / 1.0) * 100)) + '%';
                updateStatusUI(earthingGood, isSoilGood, isVoltGood);
                updateChart(voltageVal, earthingGood);
            }

            function updateStatusUI(earthingGood, soilGood, voltGood) {
                if (earthingGood) {
                    mainStatusRing.className = 'status-ring safe';
                    mainStatusIcon.className = 'fa-solid fa-check-circle';
                    mainStatusTitle.textContent = 'STATUS:';
                    mainStatusDesc.textContent = 'EARTHING GOOD';
                    mainStatusDesc.className = 'status-label good';
                    alertIndicators.classList.remove('active');
                    redLedIndicator.classList.remove('on');
                    buzzerIndicator.classList.remove('on');
                    statusHero.classList.remove('alert-mode');
                } else {
                    mainStatusRing.className = 'status-ring danger';
                    mainStatusIcon.className = 'fa-solid fa-triangle-exclamation';
                    mainStatusTitle.textContent = 'ALERT!';
                    mainStatusDesc.textContent = 'EARTHING BAD';
                    mainStatusDesc.className = 'status-label bad';
                    alertIndicators.classList.add('active');
                    redLedIndicator.classList.add('on');
                    buzzerIndicator.classList.add('on');
                    statusHero.classList.add('alert-mode');
                }
                valSoil.style.color = soilGood ? '#10b981' : '#f59e0b';
                barSoil.style.backgroundColor = soilGood ? '#10b981' : '#f59e0b';
                cardSoil.classList.toggle('warning', !soilGood);
                valVoltage.style.color = voltGood ? '#10b981' : '#ef4444';
                barVoltage.style.backgroundColor = voltGood ? '#10b981' : '#ef4444';
                cardVoltage.classList.toggle('danger', !voltGood);
            }

            function updateChart(newVolt, earthingGood) {
                const data = voltageChart.data.datasets[0].data;
                data.shift();
                data.push(newVolt);
                voltageChart.data.datasets[0].borderColor = earthingGood ? '#10b981' : '#ef4444';
                voltageChart.data.datasets[0].backgroundColor = earthingGood ? 'rgba(16,185,129,0.1)' : 'rgba(239,68,68,0.1)';
                voltageChart.update();
            }

            setInterval(updateClock, 1000);
            setInterval(fetchData, 1000);
            updateClock();
            fetchData();
        });
    </script>
</body>
</html>
"""

class EarthingMonitorHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        global sim_mode, forced_moisture, forced_voltage
        
        parsed_path = urlparse(self.path)
        
        if parsed_path.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            self.wfile.write(html_content.encode('utf-8'))
            
        elif parsed_path.path == '/set_mode':
            query = parse_qs(parsed_path.query)
            if 'mode' in query:
                sim_mode = query['mode'][0]
            self.send_response(200)
            self.end_headers()
            
        elif parsed_path.path == '/data':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            
            # Logic based on mode
            if sim_mode == "GOOD":
                voltage = 2.58 + (random.random() * 0.02 - 0.01)
                moisture = 45 + random.randint(-2, 2)
            elif sim_mode == "BAD":
                # Create a fault (e.g., low voltage or dry soil)
                voltage = 1.5 + (random.random() * 0.2) # Low voltage fault
                moisture = 10 + random.randint(0, 5)    # Dry soil fault
            else: # AUTO
                # Vary voltage slightly around 2.58
                voltage = 2.58 + (random.random() * 0.04 - 0.02)
                moisture = 40 + random.randint(-5, 5)
            
            # Logic for status
            earthing_good = (moisture >= 25) and (2.55 <= voltage <= 2.61)
            
            data = {
                "moisture": moisture,
                "voltage": voltage,
                "earthingGood": earthing_good
            }
            self.wfile.write(json.dumps(data).encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()

    def log_message(self, format, *args):
        # Suppress logging to keep output clean
        pass

if __name__ == "__main__":
    Handler = EarthingMonitorHandler
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"Mock Server running at http://localhost:{PORT}")
        print("Press Ctrl+C to stop")
        webbrowser.open(f"http://localhost:{PORT}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped.")
