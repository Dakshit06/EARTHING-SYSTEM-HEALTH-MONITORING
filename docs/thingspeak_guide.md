# ESP32 Earthing Monitor - ThingSpeak Integration Guide

## 🔧 Current Issue

**Problem:** Local dashboard shows cached old HTML with `--` values and "CONNECTING..." status.

**Root Cause:** Browser is caching the old HTML that requires Chart.js (external CDN).

## 🌐 Solution: ThingSpeak Cloud Integration

ThingSpeak provides:
- ✅ Cloud data storage
- ✅ Real-time graphs
- ✅ Mobile app access
- ✅ Email/SMS alerts
- ✅ No browser cache issues

## 📋 Setup Instructions

### Step 1: Create ThingSpeak Account
1. Go to https://thingspeak.com
2. Click "Sign Up" (free account)
3. Verify your email

### Step 2: Create a New Channel
1. Click "Channels" → "My Channels"
2. Click "New Channel"
3. Configure:
   - **Name:** Earthing Monitor
   - **Field 1:** Soil Moisture (%)
   - **Field 2:** AC Voltage (V)
   - **Field 3:** Earthing Status (0=BAD, 1=GOOD)
4. Click "Save Channel"

### Step 3: Get Your API Key
1. Go to "API Keys" tab
2. Copy **Write API Key** (looks like: `XXXXXXXXXXXXXXXX`)

### Step 4: Update ESP32 Code
Replace these lines in the code:
```cpp
const char* thingSpeakApiKey = "YOUR_WRITE_API_KEY";
unsigned long channelID = YOUR_CHANNEL_ID;
```

## 🚀 Features After Integration

### Cloud Dashboard
- View data from anywhere with internet
- Automatic graphs and charts
- Historical data storage (free: 3 months)
- Export data to CSV

### Local Dashboard  
- Simple, fast, no external dependencies
- Works offline on local network
- Real-time updates every second

## 📱 Access Your Data

**ThingSpeak Dashboard:**
- https://thingspeak.com/channels/YOUR_CHANNEL_ID

**Local Dashboard:**
- http://192.168.4.1 (when connected to ESP32 WiFi)

## ⚠️ Important Notes

1. **WiFi Mode:** ESP32 needs **Station Mode** (not AP mode) to upload to ThingSpeak
2. **Update Interval:** ThingSpeak free tier allows updates every 15 seconds
3. **Data Limit:** 3 million messages per year (free tier)

## 🔄 Recommended Setup

**Option A: Cloud Only (Recommended)**
- ESP32 connects to your home WiFi
- Uploads data to ThingSpeak
- Access from anywhere via ThingSpeak

**Option B: Hybrid (Best)**
- ESP32 connects to home WiFi
- Uploads to ThingSpeak (cloud access)
- Also serves local dashboard on same network
- Access locally at ESP32's IP address

**Option C: Local Only (Current)**
- ESP32 creates its own WiFi (AP mode)
- No internet connection
- Dashboard only works when connected to ESP32

## 📊 ThingSpeak Visualization

ThingSpeak automatically creates:
- Line charts for soil moisture
- Line charts for voltage
- Status indicator for earthing
- Customizable widgets
