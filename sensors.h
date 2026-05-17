#pragma once
#include <Arduino.h>

struct SensorData {
  float temperature   = NAN;
  float humidity      = NAN;
  int   potentiometer = -1;
  bool  button        = false;
};

void setupSensors() {}
SensorData readSensors() { return SensorData{}; }
String buildSensorJson(const SensorData& data, uint32_t nodeId) { return "{}"; }
