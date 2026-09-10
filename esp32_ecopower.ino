/* ==========================================================================
   EcoPower – ESP32 Microcontroller Firmware (SIH 2026)
   Real-Time Solar Sensing, Battery Monitoring & Priority Load Control
   ========================================================================== */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <ArduinoJson.h>

// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Flask Server Telemetry Endpoint URL (Replace IP with your laptop's local IP)
const char* serverUrl = "http://192.168.1.100:5000/api/energy";

// Pin Definitions for Relay Load Switching
#define RELAY_HIGH_PRIORITY 25 // Essential Emergency Load (Channel 1)
#define RELAY_MED_PRIORITY  26 // Controllable Cooling Fan (Channel 2)
#define RELAY_LOW_PRIORITY  27 // Non-Essential Heavy Appliance (Channel 3)
#define LDR_PIN             34 // Irradiance Sensor Analog Input

// Hardware Objects
Adafruit_INA219 ina219;

// Telemetry Variables
float busvoltage = 0;
float current_mA = 0;
float power_mW = 0;
float solar_voltage = 0;
float solar_current_A = 0;
float solar_power_W = 0;
float battery_pct = 80.0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=======================================================");
  Serial.println(" EcoPower - ESP32 Renewable Energy Controller (SIH 2026)");
  Serial.println("=======================================================");

  // Initialize Relay GPIO Pins
  pinMode(RELAY_HIGH_PRIORITY, OUTPUT);
  pinMode(RELAY_MED_PRIORITY, OUTPUT);
  pinMode(RELAY_LOW_PRIORITY, OUTPUT);

  // Default state: High & Medium ON, Low OFF
  digitalWrite(RELAY_HIGH_PRIORITY, LOW); // Active Low Relay Module
  digitalWrite(RELAY_MED_PRIORITY, LOW);
  digitalWrite(RELAY_LOW_PRIORITY, HIGH);

  // Initialize INA219 Power Sensor
  Wire.begin(21, 22);
  if (!ina219.begin()) {
    Serial.println("WARNING: INA219 Sensor not found! Running in simulated sensing mode.");
  } else {
    Serial.println("SUCCESS: INA219 Power Sensor initialized.");
  }

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected! Local IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi Connection Timeout. Operating in Serial Telemetry Mode.");
  }
}

void loop() {
  // Read INA219 Real Sensor Telemetry
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();

  if (busvoltage <= 0.1) {
    // Fallback simulation mode if solar panel is indoors or unpowered during demo
    int ldr_val = analogRead(LDR_PIN);
    solar_voltage = 12.0 + (ldr_val / 4095.0) * 4.0;
    solar_current_A = 0.5 + (ldr_val / 4095.0) * 1.8;
    solar_power_W = solar_voltage * solar_current_A;
  } else {
    solar_voltage = busvoltage;
    solar_current_A = max(0.0f, current_mA / 1000.0f);
    solar_power_W = max(0.0f, power_mW / 1000.0f);
  }

  // Calculate estimated battery percentage based on solar input
  battery_pct = min(100.0f, max(15.0f, battery_pct + (solar_power_W > 15.0 ? 0.2f : -0.1f)));

  // Local Microcontroller Priority Load Shedding Algorithm
  if (battery_pct < 25.0 || solar_power_W < 5.0) {
    // Critical state: Turn off low and medium loads, keep essential load active
    digitalWrite(RELAY_HIGH_PRIORITY, LOW);  // ON
    digitalWrite(RELAY_MED_PRIORITY, HIGH);  // OFF
    digitalWrite(RELAY_LOW_PRIORITY, HIGH);  // OFF
    Serial.println("SMART ACTION: Low Power Detected. Shedding non-essential loads.");
  } else if (solar_power_W >= 30.0 && battery_pct >= 60.0) {
    // Abundant state: Turn ON all loads including low priority
    digitalWrite(RELAY_HIGH_PRIORITY, LOW);  // ON
    digitalWrite(RELAY_MED_PRIORITY, LOW);   // ON
    digitalWrite(RELAY_LOW_PRIORITY, LOW);   // ON
    Serial.println("SMART ACTION: Solar Abundant. All loads enabled.");
  } else {
    // Standard state: Essential & Medium loads active
    digitalWrite(RELAY_HIGH_PRIORITY, LOW);  // ON
    digitalWrite(RELAY_MED_PRIORITY, LOW);   // ON
    digitalWrite(RELAY_LOW_PRIORITY, HIGH);  // OFF
  }

  // Send Telemetry Payload via HTTP POST to Flask Backend
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    doc["voltage"] = solar_voltage;
    doc["current"] = solar_current_A;
    doc["power"] = solar_power_W;
    doc["battery_pct"] = battery_pct;
    doc["source"] = "esp32_hardware";

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      Serial.printf("Telemetry HTTP POST Success! Code: %d\n", httpResponseCode);
    } else {
      Serial.printf("HTTP POST Failed Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    // Output JSON via Serial Monitor for local tethered serial communication
    StaticJsonDocument<256> doc;
    doc["voltage"] = solar_voltage;
    doc["current"] = solar_current_A;
    doc["power"] = solar_power_W;
    doc["battery_pct"] = battery_pct;
    doc["source"] = "esp32_serial";
    serializeJson(doc, Serial);
    Serial.println();
  }

  delay(3000); // 3 Second Transmission Interval
}
