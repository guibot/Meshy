#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#ifdef ENABLE_AHT10
#include <Adafruit_AHTX0.h>
static Adafruit_AHTX0 _aht;
static bool           _ahtOk = false;
#endif

// ── Types ─────────────────────────────────────────────────────────────────────

struct SensorData {
  float temperature   = NAN;   // °C  — NAN if disabled or failed
  float humidity      = NAN;   // %RH — NAN if disabled or failed
  int   potentiometer = -1;    // 0–4095 ADC raw; -1 if disabled
  bool  button        = false;
};

// ── Setup ─────────────────────────────────────────────────────────────────────

void setupSensors() {
#ifdef ENABLE_AHT10
  Wire.begin(PIN_SDA, PIN_SCL);
  _ahtOk = _aht.begin();
  Serial.printf("[Sensors] AHT10 %s\n", _ahtOk ? "OK" : "FAILED — check wiring");
#endif

#ifdef ENABLE_POTENTIOMETER
  pinMode(ANALOG_PIN, INPUT);
  Serial.printf("[Sensors] Potentiometer on GPIO%d\n", ANALOG_PIN);
#endif

#ifdef ENABLE_BUTTON
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.printf("[Sensors] Button on GPIO%d (active LOW)\n", BUTTON_PIN);
#endif
}

// ── Read ──────────────────────────────────────────────────────────────────────

SensorData readSensors() {
  SensorData data;

#ifdef ENABLE_AHT10
  if (_ahtOk) {
    sensors_event_t hum, temp;
    _aht.getEvent(&hum, &temp);
    data.temperature = temp.temperature;
    data.humidity    = hum.relative_humidity;
  }
#endif

#ifdef ENABLE_POTENTIOMETER
  data.potentiometer = analogRead(ANALOG_PIN);
#endif

#ifdef ENABLE_BUTTON
  data.button = (digitalRead(BUTTON_PIN) == LOW);
#endif

  return data;
}

// ── JSON builder ──────────────────────────────────────────────────────────────

String buildSensorJson(const SensorData& data, uint32_t nodeId) {
  StaticJsonDocument<512> doc;
  doc["node_id"]   = String(nodeId, HEX);
  doc["location"]  = NODE_LOCATION;
  doc["mode"]      = "node";
  doc["timestamp"] = millis();   // relative ms since boot; root stamps real time on receipt

  JsonObject sensors = doc.createNestedObject("sensors");

#ifdef ENABLE_AHT10
  if (!isnan(data.temperature)) sensors["temperature"] = data.temperature;
  if (!isnan(data.humidity))    sensors["humidity"]    = data.humidity;
#endif

#ifdef ENABLE_POTENTIOMETER
  if (data.potentiometer >= 0)  sensors["potentiometer"] = data.potentiometer;
#endif

#ifdef ENABLE_BUTTON
  sensors["button"] = data.button;
#endif

  String out;
  serializeJson(doc, out);
  return out;
}
