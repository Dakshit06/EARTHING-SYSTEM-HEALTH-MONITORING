# ESP32 Smart Earthing Monitoring System
## Detailed Pin Configuration Guide

---

## 📌 Complete Pin Mapping Table

| Component | Component Pin | ESP32 Pin | Wire Color |
|-----------|---------------|-----------|------------|
| **OLED Display** | GND | GND | Black |
| | VDD | 3.3V | Red |
| | SDA | GPIO 21 | Blue |
| | SCK/SCL | GPIO 22 | Yellow |
| **ADS1115** | GND | GND | Black |
| | VDD | 3.3V | Red |
| | SDA | GPIO 21 | Blue |
| | SCL | GPIO 22 | Yellow |
| | ADDR | GND | Black |
| **Soil Sensor** | GND | GND | Black |
| | VCC | 3.3V | Red |
| | AO | ADS1115 A0 | Green |
| **Voltage Sensor** | GND | GND | Black |
| | VCC | 3.3V | Red |
| | OUT | ADS1115 A1 | Orange |
| **Buzzer** | (-) | GND | Black |
| | (+) | GPIO 27 | Purple |
| **Red LED** | Cathode (-) | GND | Black |
| | Anode (+) | GPIO 26 | Brown |
| **220Ω Resistor** | End 1 | GPIO 26 | - |
| | End 2 | LED Anode | - |

---

## 🔌 Component-Wise Wiring Details

### 1. OLED Display (0.96" SSD1306 I2C)

```
OLED Display          ESP32
┌───────────┐        ┌─────────┐
│ GND ──────┼────────│ GND     │
│ VDD ──────┼────────│ 3.3V    │
│ SDA ──────┼────────│ GPIO 21 │
│ SCK ──────┼────────│ GPIO 22 │
└───────────┘        └─────────┘
```

> [!IMPORTANT]
> - Use **3.3V only** (NOT 5V - can damage display)
> - SCK = SCL (same thing, different label)
> - I2C Address: **0x3C** (or try 0x3D)

---

### 2. ADS1115 (16-bit I2C ADC)

```
ADS1115               ESP32
┌────────────┐       ┌─────────┐
│ VDD ───────┼───────│ 3.3V    │
│ GND ───────┼───────│ GND     │
│ SDA ───────┼───────│ GPIO 21 │
│ SCL ───────┼───────│ GPIO 22 │
│ ADDR ──────┼───────│ GND     │  ← Sets address 0x48
│            │       └─────────┘
│ A0 ────────┼─── Soil Sensor AO
│ A1 ────────┼─── Voltage Sensor OUT
│ A2 ────────┼─── (unused)
│ A3 ────────┼─── (unused)
└────────────┘
```

> [!NOTE]
> - ADDR pin to GND = I2C address **0x48**
> - ADDR pin to VDD = I2C address **0x49**

---

### 3. Soil Moisture Sensor

```
Soil Sensor           Connection
┌────────────┐
│ VCC ───────┼─────── ESP32 3.3V
│ GND ───────┼─────── ESP32 GND
│ AO ────────┼─────── ADS1115 A0
│ DO ────────┼─────── (not used)
└────────────┘
```

> [!TIP]
> - AO = Analog Output (variable 0-3.3V)
> - DO = Digital Output (not needed for this project)
> - Use capacitive sensor for longer life

---

### 4. Voltage Sensor Module (ZMPT101B or similar)

```
Voltage Sensor        Connection
┌────────────┐
│ VCC ───────┼─────── ESP32 3.3V
│ GND ───────┼─────── ESP32 GND
│ OUT ───────┼─────── ADS1115 A1
└────────────┘
```

> [!CAUTION]
> - Never connect AC directly to ESP32!
> - Use proper isolation voltage sensor module
> - Module must output 0-3.3V analog signal

---

### 5. Buzzer (5V Active Buzzer)

```
Buzzer                ESP32
┌────────────┐       ┌─────────┐
│ (+) ───────┼───────│ GPIO 27 │
│ (-) ───────┼───────│ GND     │
└────────────┘       └─────────┘
```

> [!NOTE]
> - Active buzzer: sounds when HIGH signal
> - Passive buzzer: needs PWM signal (tone)

---

### 6. Red LED with Resistor

```
ESP32 GPIO 26 ───┤220Ω├─── LED (+) ─── LED (-) ─── GND
```

Detailed:
```
GPIO 26            220Ω Resistor         Red LED
   │                   │                   │
   └─────────[////]────┴────►|────────────┘
                            Anode  Cathode
                             (+)     (-)
                                      │
                                     GND
```

> [!IMPORTANT]
> - Always use resistor to protect LED
> - 220Ω-330Ω for 3.3V supply
> - LED forward voltage ~2V

---

## 🔗 I2C Bus Connections (Parallel Wiring)

Both OLED and ADS1115 share the same I2C bus:

```
                    ┌──────────────┐
                    │   ESP32      │
                    │              │
             ┌──────│ GPIO 21 SDA  │──────┐
             │      │              │      │
             │ ┌────│ GPIO 22 SCL  │────┐ │
             │ │    │              │    │ │
             │ │ ┌──│ 3.3V         │──┐ │ │
             │ │ │  │              │  │ │ │
             │ │ │ ┌│ GND          │┐ │ │ │
             │ │ │ │└──────────────┘│ │ │ │
             │ │ │ │                │ │ │ │
     ┌───────┴─┴─┴─┴─┐      ┌──────┴─┴─┴─┴───┐
     │    OLED       │      │   ADS1115      │
     │               │      │                │
     │ SDA  SCL VDD GND     │ SDA SCL VDD GND│
     └───────────────┘      └────────────────┘
```

---

## 📋 ESP32 DevKit V1 Pinout Reference

```
         ┌─────────────────┐
         │    ESP32        │
         │   DevKit V1     │
         │                 │
    3V3 ─┤ 3V3       VIN  ├─ 5V (from USB)
    GND ─┤ GND       GND  ├─ GND
GPIO 15 ─┤ D15       D13  ├─ GPIO 13
GPIO  2 ─┤ D2        D12  ├─ GPIO 12
GPIO  4 ─┤ D4        D14  ├─ GPIO 14
GPIO 16 ─┤ RX2       D27  ├─ GPIO 27 ◄── BUZZER
GPIO 17 ─┤ TX2       D26  ├─ GPIO 26 ◄── RED LED
GPIO  5 ─┤ D5        D25  ├─ GPIO 25
GPIO 18 ─┤ D18       D33  ├─ GPIO 33
GPIO 19 ─┤ D19       D32  ├─ GPIO 32
GPIO 21 ─┤ D21       D35  ├─ GPIO 35 (input only)
GPIO  3 ─┤ RX0       D34  ├─ GPIO 34 (input only)
GPIO  1 ─┤ TX0       VN   ├─ GPIO 39 (input only)
GPIO 22 ─┤ D22       VP   ├─ GPIO 36 (input only)
GPIO 23 ─┤ D23       EN   ├─ Enable
         │                 │
         │   [USB PORT]    │
         └─────────────────┘

◄── Used Pins:
    GPIO 21 = I2C SDA (OLED + ADS1115)
    GPIO 22 = I2C SCL (OLED + ADS1115)
    GPIO 26 = Red LED
    GPIO 27 = Buzzer
```

---

## ⚡ Power Distribution

```
ESP32 3.3V ───┬─── OLED VDD
              ├─── ADS1115 VDD
              ├─── Soil Sensor VCC
              └─── Voltage Sensor VCC

ESP32 GND ────┬─── OLED GND
              ├─── ADS1115 GND
              ├─── ADS1115 ADDR (for 0x48)
              ├─── Soil Sensor GND
              ├─── Voltage Sensor GND
              ├─── Buzzer (-)
              └─── LED Cathode (-)
```

---

## ✅ Wiring Checklist

Before powering on, verify:

- [ ] OLED SDA → ESP32 GPIO 21
- [ ] OLED SCL → ESP32 GPIO 22
- [ ] OLED VDD → ESP32 3.3V (NOT 5V!)
- [ ] OLED GND → ESP32 GND
- [ ] ADS1115 SDA → ESP32 GPIO 21
- [ ] ADS1115 SCL → ESP32 GPIO 22
- [ ] ADS1115 VDD → ESP32 3.3V
- [ ] ADS1115 GND → ESP32 GND
- [ ] ADS1115 ADDR → ESP32 GND
- [ ] Soil Sensor AO → ADS1115 A0
- [ ] Soil Sensor VCC → 3.3V
- [ ] Soil Sensor GND → GND
- [ ] Voltage Sensor OUT → ADS1115 A1
- [ ] Voltage Sensor VCC → 3.3V
- [ ] Voltage Sensor GND → GND
- [ ] Buzzer (+) → GPIO 27
- [ ] Buzzer (-) → GND
- [ ] LED Anode (+) → 220Ω → GPIO 26
- [ ] LED Cathode (-) → GND

---

## 🛠️ Code File

Upload `esp32_earthing_monitor.ino` to your ESP32 using Arduino IDE.

Required Libraries:
- `Adafruit ADS1X15`
- `Adafruit SSD1306`
- `Adafruit GFX`

---

## 📝 Quick Reference Card

| Function | ESP32 Pin | Note |
|----------|-----------|------|
| I2C SDA | GPIO 21 | Shared bus |
| I2C SCL | GPIO 22 | Shared bus |
| Buzzer | GPIO 27 | Active HIGH |
| Red LED | GPIO 26 | Via 220Ω resistor |
| Power | 3.3V | All components |
| Ground | GND | Common ground |
