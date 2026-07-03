# IoT Power Monitor (ESP32 & ThingSpeak)

## Overview
This project is an IoT-based power monitoring system developed for an ESP32 microcontroller. It reads analog voltage and current signals, processes them to calculate key electrical parameters, and automatically logs the data to a ThingSpeak dashboard over Wi-Fi.

## Features
- **Accurate AC Measurements:** Implements a continuous 100ms sampling window for reliable True RMS calculations.
- **Dynamic Calibration:** Real-time DC offset recalculation for both voltage and current sensors to ensure precision.
- **Signal Analysis:** Zero-crossing detection algorithm to calculate the grid frequency.
- **IoT Integration:** Non-blocking Wi-Fi connection and automated HTTP POST requests to ThingSpeak.

## Calculated Parameters
- RMS Voltage (V)
- RMS Current (A)
- Active Power (W)
- Apparent Power (VA)
- Power Factor
- Frequency (Hz)

## Hardware & Pinout Mapping
- **Microcontroller:** ESP32
- **Sensors:** Analog Voltage Transducer & Analog Current Transducer

| ESP32 Pin | Connection | Description |
| :--- | :--- | :--- |
| `3V3` | Sensors VCC | Power supply for the sensors |
| `GND` | Sensors GND | Common ground |
| `GPIO 34` | Voltage Sensor | Analog input for voltage readings (ADC 12-bit) |
| `GPIO 35` | Current Sensor | Analog input for current readings (ADC 12-bit) |

## Setup & Installation
1. Clone this repository to your local machine.
2. Open the project environment (developed in [PlatformIO](https://platformio.org/)).
3. Ensure the following libraries are installed:
   - `WiFi.h` (Standard ESP32 library)
   - `ThingSpeak` (by MathWorks)
4. In `main.cpp` (or `.ino`), update the configuration placeholders:
   - `ssid` and `password` with your local Wi-Fi credentials.
   - `myChannelNumber` and `myWriteAPIKey` with your ThingSpeak channel details.
5. Compile and flash the code to your ESP32.

## Security Note
Never commit your real Wi-Fi passwords or ThingSpeak API keys to a public repository. Always use placeholders or a `.gitignore` approach for sensitive credentials before pushing.