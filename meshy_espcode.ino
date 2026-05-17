#include <Preferences.h>
#include "config.h"
#include "mqtt_stub.h"
#include "mesh_module.h"
#include "sensors.h"
#include "display.h"
#include "webserver.h"

Preferences prefs;
bool isRoot = false;
String wifiIP = "";

void detectAndSaveMode() {
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  delay(100);
  bool buttonHeld = (digitalRead(MODE_BUTTON_PIN) == LOW);
  prefs.begin("meshy", false);
  if (buttonHeld) {
    isRoot = true;
    prefs.putBool("isRoot", true);
    Serial.println("[Mode] Button held — setting ROOT mode");
  } else {
    isRoot = prefs.getBool("isRoot", false);
  }
  prefs.end();
  Serial.printf("[Mode] Running as: %s\n", isRoot ? "ROOT" : "NODE");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[Boot] meshy_espcode starting...");
  detectAndSaveMode();
  setupMesh(isRoot);
  if (isRoot) {
    setupDisplay();
    setupWebServer();
    setupMQTT();
  } else {
    setupSensors();
  }
  Serial.println("[Boot] Setup complete");
}

static unsigned long lastSendMs = 0;

void loop() {
  updateMesh();
  if (isRoot) {
    updateDisplay(wifiIP);
  } else {
    if (millis() - lastSendMs >= SEND_INTERVAL) {
      lastSendMs = millis();
      SensorData data = readSensors();
      String json = buildSensorJson(data, getMeshNodeId());
      Serial.printf("[Node] Sending: %s\n", json.c_str());
      sendMeshData(json);
    }
  }
}
