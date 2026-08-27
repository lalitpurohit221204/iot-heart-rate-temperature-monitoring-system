# IoT-Based Heart Rate & Temperature Monitoring System

A real-time patient vitals monitor built on the ESP8266 NodeMCU. It reads heart rate and body temperature, streams the data to the cloud for live visualization, and automatically fires an SMS alert the moment a reading falls outside a safe range — closing the loop between sensing and action without any human watching a screen.

Built as a minor project by **Group 1, Department of Electrical Engineering, Shri G.S. Institute of Technology and Science.**

---

## Skills Demonstrated

- **Embedded C/C++** — real-time sensor sampling and control logic on a resource-constrained microcontroller
- **Signal processing** — dynamic, self-adjusting peak-detection algorithm for noisy analog input
- **IoT & networking** — WiFi connectivity, REST/HTTP calls, and cloud telemetry from an embedded device
- **Third-party API integration** — Twilio (SMS) and ThingSpeak (cloud dashboard) via HTTPS
- **System design** — end-to-end pipeline from sensor to cloud to alert, with a clear fail-safe threshold

---

## Overview

Continuous manual monitoring of patient vitals is impractical in hospitals, elder care, and remote settings — someone has to be watching, all the time. This project automates that:

**Sensors → ESP8266 NodeMCU → ThingSpeak Cloud → Dashboard → Twilio SMS Alert**

- Heart rate and temperature are sampled in real time
- Data is pushed to a ThingSpeak channel for live visualization
- If heart rate goes below 50 or above 120 BPM, the system automatically triggers an emergency SMS alert via Twilio, backed by a local buzzer for anyone nearby

---

## Hardware Components

| Component | Purpose |
|---|---|
| ESP8266 NodeMCU V3 | Microcontroller + WiFi |
| Analog Pulse/Heartbeat Sensor | Heart rate (BPM) measurement |
| DS18B20 Temperature Sensor | Body temperature (fever detection) |
| SSD1306 OLED Display | Live on-device readout |
| Buzzer | Audible emergency alert |
| Status LED | System/heartbeat indicator |
| 4.7kΩ Resistor | Pull-up for DS18B20 |

## Software / Services

- **Arduino IDE** — code development and upload
- **ThingSpeak** — cloud data logging and real-time visualization
- **Twilio API** — emergency SMS alerts

---

## How It Works

1. **Data Collection** — Pulse sensor and DS18B20 continuously sample vitals.
2. **Peak Detection** — Heartbeats are detected using a *dynamic, self-adjusting threshold* (rise relative to the previous reading), rather than a fixed ADC window, so it adapts to sensor placement, lighting, and skin tone.
3. **BPM Calculation** — Beats are counted over a 20-second window and scaled to beats-per-minute.
4. **Cloud Logging** — Readings are pushed to a ThingSpeak channel every cycle.
5. **Emergency Alert** — If BPM is outside the 50–120 safe range, the buzzer sounds locally and an SMS is sent via Twilio with the abnormal reading.

## Dashboard

Live heart rate chart, current BPM widget, and alert indicator hosted on ThingSpeak:

![Dashboard](docs/dashboard.png)

The dashboard provides real-time visualization of heart rate, current BPM, and alert status, allowing caregivers to monitor patient health remotely.

Temperature is logged to the same channel (Field 2) and visualized alongside heart rate:

![Temperature](docs/temperature.png)

*Note: heart rate and temperature screenshots above were captured in separate testing sessions during development.*

## Emergency Alert in Action

Real Twilio SMS messages triggered by abnormal readings during testing:

![SMS Alert](docs/sms_alert.png)

---

## Circuit Diagram

![Circuit Diagram](docs/circuit_diagram.png)

> Note: this diagram is being updated — an earlier draft mislabeled the heartbeat sensor as a MAX30102 (I2C). The project uses a plain analog pulse sensor on pin A0, matching `heart_rate_monitor.ino`.

---

## Getting Started

1. Clone this repo
2. Open `heart_rate_monitor.ino` in Arduino IDE
3. Install required libraries: `ESP8266WiFi`, `ESP8266HTTPClient`, `ThingSpeak`, `Adafruit_SSD1306`, `Adafruit_GFX`, `OneWire`, `DallasTemperature` (the `WiFiClientSecure` class used for the Twilio HTTPS request ships with the ESP8266 board package, no separate install needed)
4. Replace the placeholder credentials at the top of the file with your own:
   - WiFi SSID / password
   - ThingSpeak channel number + Write API key
   - Twilio SID / Auth Token / phone numbers
5. Flash to your ESP8266 NodeMCU and open the Serial Monitor at 115200 baud

> ⚠️ Security note: This repository uses placeholder credentials. For your own deployment, store real WiFi, ThingSpeak, and Twilio credentials in a separate secrets.h file and add it to .gitignore — never commit credentials directly to version control.

---

## Key Design Decision: Dynamic Threshold Peak Detection

An earlier version used a **fixed ADC threshold** — counting a beat only when the raw sensor value fell inside a narrow fixed band. This proved unreliable across users, finger pressure, and ambient light, and often undercounted real beats.

The current version detects a beat as a **rise relative to the previous sample** instead, self-adjusting to the live baseline signal for far more consistent BPM readings.

---

## Lessons Learned

- A dynamic, relative threshold generalizes far better than a fixed calibration constant — worth defaulting to for any noisy analog sensor.
- Keeping hardware documentation (circuit diagram) in sync with firmware is easy to let slip during rapid iteration; next revision will generate the diagram from a single source of truth.
- Given more time, the next iteration would add SpO₂ sensing and move anomaly detection off simple thresholds toward a lightweight on-device model to cut false alerts.

---

## Future Scope

- Add SpO₂, ECG, and blood pressure sensing
- GSM (SIM800L) fallback for alerts without WiFi
- AI-based anomaly detection to reduce false alerts
- Wearable / compact form factor
- Companion mobile app
- Multi-patient monitoring dashboard for hospital use

---

## Team

Lalit Purohit · Harsh Gupta · Krish Gupta · Karan Sankhla · Karan Rathor · Jatin Gaur
