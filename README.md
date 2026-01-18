# Earthing System Health Monitoring System ⚡🌍

A smart IoT-based system to monitor the effectiveness of electrical earthing (grounding) in real-time. This project uses an ESP32 to measure **Soil Moisture** and **Leakage Voltage**, hosting a premium web dashboard for monitoring.

![Project Dashboard](dashboard_preview.png)

## 🚀 Features
- **Real-Time Monitoring**: Live visualization of Soil Moisture (%) and Leakage Voltage (V).
- **Web Dashboard**: Premium, dark-mode glassmorphism UI hosted directly on the ESP32.
- **WiFi Access Point**: Connects to your phone/laptop via "EarthingGuard" hotspot.
- **OLED Display**: On-device status summary.
- **Hardware Alerts**: Buzzer & LED alerts for unsafe conditions (Low Moisture or High Voltage).
- **Safety Logic**:
  - **Safe**: Moisture > 25% AND Voltage 2.55V - 2.61V.
  - **Alert**: Triggers if conditions are not met.

## 🛠️ Hardware Requirements
- **Microcontroller**: ESP32 Dev Module
- **ADC**: ADS1115 (16-bit)
- **Display**: 0.96" OLED (I2C, SSD1306)
- **Sensors**:
  - Soil Moisture Sensor (Capacitive/Resistive)
  - ZMPT101B AC Voltage Sensor (or similar analog output)
- **Output**:
  - Active Buzzer
  - LED (Red) + 220Ω Resistor
- Breadboard & Jumper Wires

## 🔌 Wiring Diagram

| Component | ESP32 Pin | Note |
|-----------|-----------|------|
| **OLED SDA** | GPIO 21 | I2C |
| **OLED SCL** | GPIO 22 | I2C |
| **ADS1115 SDA** | GPIO 21 | Shared I2C |
| **ADS1115 SCL** | GPIO 22 | Shared I2C |
| **Soil Sensor** | ADS1115 A0 | Analog In |
| **Volt Sensor** | ADS1115 A1 | Analog In |
| **LED** | GPIO 26 | Digital Out |
| **Buzzer** | GPIO 27 | Digital Out |

## 💻 Software & Installation

### 1. Prerequisites
Install **Arduino IDE** and add the **ESP32 Board Manager**.

### 2. Required Libraries
Install these via Library Manager:
- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `Adafruit ADS1X15`

### 3. Uploading Code
1. Open [`ESP32_Earthing_WiFi.ino`](ESP32_Earthing_WiFi.ino) in Arduino IDE.
2. Select Board: **DOIT ESP32 DEVKIT V1**.
3. Connect ESP32 via USB and click **Upload**.

*Note: The HTML/CSS/JS files are already embedded in the Arduino sketch, so you don't need to upload SPIFFS data separately.*

## 📱 How to Use
1. Power on the system.
2. Connect your phone/laptop WiFi to:
   - **SSID**: `EarthingGuard`
   - **Password**: `12345678`
3. Open a browser and navigate to:
   - **http://192.168.4.1**
4. View the live dashboard!

## 📂 Project Structure
- `ESP32_Earthing_WiFi.ino`: **Main Firmware**. Hosts WiFi, Web Server, and Logic.
- `web_interface/`: Source files for the dashboard (for development/editing).
  - `index.html`: Dashboard structure.
  - `style.css`: Styling.
  - `script.js`: Frontend logic.
- `test_*.ino`: Individual component test sketches.

## 🤝 Contributing
Feel free to fork this repository and submit pull requests.

## 📜 License
This project is open-source.
System to check the healthiness of earthing system and alert staff in case of any malfunction
