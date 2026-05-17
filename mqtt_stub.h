#pragma once
#include <Arduino.h>

void setupMQTT() {
  Serial.println("[MQTT] stub — phase 2");
}

inline void publishMQTT(const char* topic, const char* payload) {
  // no-op — implement in phase 2 with PubSubClient
}
