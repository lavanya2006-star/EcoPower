# Bill of Materials (BOM) & Component Specifications

The EcoPower prototype uses readily available, budget-friendly components that any undergraduate engineering student team can source from local electronics stores or online platforms.

| Component Name | Description / Specification | Quantity | Primary Function | Approx Cost (₹) |
| :--- | :--- | :---: | :--- | :---: |
| **ESP32 Dev Module** | NodeMCU ESP32-WROOM-32 (38 Pin) | 1 | Microcontroller, WiFi client & Relay controller | ₹350 |
| **Solar Panel** | 12V 5W–10W Monocrystalline / Polycrystalline | 1 | Renewable Solar DC Generation | ₹450 |
| **Battery Pack** | 12V 7Ah Sealed Lead-Acid or LiFePO4 Pack | 1 | Energy Storage Bank | ₹650 |
| **Solar Charge Controller** | 12V 10A PWM / TP4056 BMS Module | 1 | Overcharge & Deep Discharge Protection | ₹150 |
| **INA219 Sensor** | I2C High-Side DC Current & Voltage Sensor | 1 | High-precision $V, I, P$ Telemetry Measurement | ₹180 |
| **Relay Module** | 4-Channel 5V Optocoupler Relay Module | 1 | Priority Load Switching (High/Med/Low) | ₹140 |
| **Loads (Demo)** | 12V LED Strip, 12V DC Fan, 10W Power Resistor | 3 | Physical representation of High, Med, Low loads | ₹200 |
| **OLED Display** | 0.96 inch I2C SSD1306 OLED Display | 1 | Local on-board status output | ₹160 |
| **LDR Sensor** | Light Dependent Resistor module | 1 | Irradiance monitoring | ₹40 |
| **Breadboard & Wires** | 830-point Breadboard + Male-Female Jumpers | 1 set | Prototyping connections | ₹120 |
| **Total Estimated Hardware Cost** | | | **Complete Hardware Prototype** | **~₹2,440** |

---

## Detailed Specifications

### 1. ESP32 Microcontroller
- **Dual Core 32-bit Tensilica LX6**, operating at up to 240 MHz.
- **Built-in 2.4 GHz WiFi (802.11 b/g/n)** and Bluetooth 4.2 BLE.
- Operates at 3.3V logic level; powered via Micro-USB or 5V VIN pin.

### 2. INA219 Power Sensor Module
- **Communication Protocol:** I2C (Default address `0x40`).
- **Voltage Range:** 0V to +26V DC.
- **Current Measurement Range:** Up to $\pm3.2\text{A}$ (0.1 ohm shunt resistor).
- Accuracy: $\pm1\%$ precision for voltage and current.

### 3. Priority Load Relays
- **Channel 1 (High Priority):** Connected to GPIO 25 -> 12V Emergency LED Indicator.
- **Channel 2 (Medium Priority):** Connected to GPIO 26 -> 12V DC Ventilation Fan.
- **Channel 3 (Low Priority):** Connected to GPIO 27 -> 12V Resistor Load / Heavy Appliance Demo.
