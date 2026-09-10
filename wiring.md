# EcoPower Wiring & Pin Mapping Guide

This document provides the pin layout, breadboard wiring instructions, and schematic representation for connecting the ESP32 microcontroller, INA219 power sensor, relay module, OLED display, and solar panel array.

## Complete Pin Mapping Table

| ESP32 Pin | Connected Component | Component Pin | Description |
| :--- | :--- | :--- | :--- |
| **5V / VIN** | Relay Module, INA219, OLED | VCC / 5V | System 5V Power Rail |
| **GND** | All Modules, Solar Ground, Battery (-) | GND / Common Ground | Shared Common Ground |
| **GPIO 21** | INA219 Sensor, OLED Display | SDA | I2C Data Line |
| **GPIO 22** | INA219 Sensor, OLED Display | SCL | I2C Clock Line |
| **GPIO 25** | Relay Channel 1 | IN1 | High Priority Load Control (Essential) |
| **GPIO 26** | Relay Channel 2 | IN2 | Medium Priority Load Control (Fan) |
| **GPIO 27** | Relay Channel 3 | IN3 | Low Priority Load Control (Heavy Load) |
| **GPIO 34 (ADC)** | LDR Light Sensor | OUT | Analog Sunlight Irradiance Signal |

---

## Power Connections & Sensor Wiring

### INA219 Power Measurement Wiring
1. Connect **VIN+** of INA219 module to the **(+) Positive terminal of Solar Panel**.
2. Connect **VIN-** of INA219 module to the **(+) Charge Controller Solar Input / Battery Positive (+)**.
3. Connect **GND** of INA219 module to Common Ground.
4. Connect **SDA** to ESP32 **GPIO 21** and **SCL** to ESP32 **GPIO 22**.

### Relay Load Connection Circuit
1. **Relay VCC** -> ESP32 **5V**.
2. **Relay GND** -> ESP32 **GND**.
3. **Relay IN1 (High Priority)** -> **GPIO 25**.
4. **Relay IN2 (Medium Priority)** -> **GPIO 26**.
5. **Relay IN3 (Low Priority)** -> **GPIO 27**.
6. Each relay switch terminal (NO - Normally Open) connects 12V power from the battery rail to its respective load (High Priority LED, Medium Fan, Low Power Resistor).

---

## Safe Prototyping Checklist
- [x] Verify common ground between ESP32, INA219, and 12V battery power supply.
- [x] Ensure INA219 DC voltage never exceeds +26V DC.
- [x] Use optocoupler-isolated relay modules to protect ESP32 GPIO pins from inductive feedback.
- [x] Double-check positive and negative polarities before connecting battery storage.
