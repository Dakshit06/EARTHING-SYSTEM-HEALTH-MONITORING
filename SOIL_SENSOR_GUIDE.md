# Soil Sensor Module - Earthing Project

## Overview
This soil sensor module monitors ground conductivity to ensure proper earthing/grounding for safety systems.

---

## Components Required

| Component | Quantity | Notes |
|-----------|----------|-------|
| Arduino Uno/Nano | 1 | Microcontroller |
| Capacitive Soil Moisture Sensor | 1 | Recommended for durability |
| Red LED | 1 | Poor earthing indicator |
| Yellow LED | 1 | Moderate earthing indicator |
| Green LED | 1 | Good earthing indicator |
| Buzzer (5V) | 1 | Alert for poor earthing |
| 220Ω Resistors | 3 | For LEDs |
| Jumper Wires | As needed | Connections |
| Breadboard | 1 | Prototyping |

---

## Circuit Diagram

```
                    ARDUINO UNO
                   ┌───────────┐
                   │           │
    Soil Sensor ───┤ A0        │
                   │           │
    Red LED ───────┤ D5        │
    Yellow LED ────┤ D6        │
    Green LED ─────┤ D7        │
    Buzzer ────────┤ D8        │
                   │           │
                   │    5V ────┼──── Sensor VCC
                   │    GND ───┼──── Sensor GND, LEDs(-), Buzzer(-)
                   └───────────┘
```

## Wiring Connections

### Soil Sensor
| Sensor Pin | Arduino Pin |
|------------|-------------|
| VCC | 5V |
| GND | GND |
| AO (Analog Out) | A0 |

### LEDs (each with 220Ω resistor)
| LED | Arduino Pin |
|-----|-------------|
| Red (+) | D5 |
| Yellow (+) | D6 |
| Green (+) | D7 |
| All (-) | GND |

### Buzzer
| Buzzer | Arduino Pin |
|--------|-------------|
| (+) | D8 |
| (-) | GND |

---

## Status Indicators

| Sensor Value | Status | LED | Action |
|--------------|--------|-----|--------|
| 0 - 300 | Poor Earthing | 🔴 Red | Buzzer Alert |
| 300 - 600 | Moderate | 🟡 Yellow | Monitor |
| 600 - 1023 | Good Earthing | 🟢 Green | Optimal |

---

## Calibration

1. **Dry Soil Test**: Insert sensor in dry soil, note reading (~0-300)
2. **Wet Soil Test**: Insert sensor in wet soil, note reading (~600-1023)
3. **Adjust Thresholds**: Modify `THRESHOLD_POOR` and `THRESHOLD_MODERATE` in code

---

## Upload Instructions

1. Open `soil_sensor.ino` in Arduino IDE
2. Select Board: **Arduino Uno** (or your board)
3. Select correct COM port
4. Click **Upload**
5. Open Serial Monitor (9600 baud) to view readings

---

## Safety Notes

> [!CAUTION]
> This is a monitoring system only. For actual earthing installations, consult a licensed electrician.

> [!IMPORTANT]
> Ensure proper insulation when deploying near electrical earthing points.
