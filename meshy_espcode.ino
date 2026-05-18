#include "config.h"
#include "mesh_module.h"
#include "mqtt_stub.h"
#include "sensors.h"
#include "display.h"
#include "webserver.h"

// ── Globals ───────────────────────────────────────────────────────────────────

bool   isRoot = false;
String wifiIP = "";

// ── Mode detection ────────────────────────────────────────────────────────────

void detectAndSaveMode() {
#ifndef ENABLE_DISPLAY
  // Hold BUTTON_PIN on reset → ROOT for this session only (no NVS persistence)
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(100);
  isRoot = (digitalRead(BUTTON_PIN) == LOW);
  if (isRoot) Serial.println("[Mode] Button held — ROOT mode");
#else
  isRoot = true;  // display build always ROOT (M5Core2)
#endif
  Serial.printf("[Mode] Running as: %s\n", isRoot ? "ROOT" : "NODE");
}

// ── WiFi (root only) ──────────────────────────────────────────────────────────

#ifdef ENABLE_WEBSERVER
#include <WiFi.h>

void setupWiFi() {
  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiIP = WiFi.localIP().toString();
    Serial.printf("\n[WiFi] Connected — http://%s\n", wifiIP.c_str());
  } else {
    Serial.println("\n[WiFi] Connection failed — web server unavailable");
  }
}
#else
void setupWiFi() {}
#endif

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  // ESP32-C3 uses USB-CDC; wait up to 3s for the port to open
  unsigned long t = millis();
  while (!Serial && millis() - t < 3000);
  delay(100);

  Serial.println("\n[Boot] meshy_espcode starting...");

  detectAndSaveMode();

  if (isRoot) {
    setupWiFi();      // connect STA first so mesh AP inherits the same WiFi channel
    setupMesh(true);
    setupDisplay();
    setupWebServer();
    setupMQTT();
  } else {
    setupMesh(false);
    setupSensors();
  }

  Serial.println("[Boot] Setup complete");
}

// ── Loop ──────────────────────────────────────────────────────────────────────

static unsigned long _lastSendMs = 0;

void loop() {
  updateMesh();   // drives painlessMesh scheduler — must be called every loop

  if (isRoot) {
    updateDisplay(wifiIP);
  } else {
    if (millis() - _lastSendMs >= SEND_INTERVAL) {
      _lastSendMs = millis();
      SensorData data = readSensors();
      String json = buildSensorJson(data, getMeshNodeId());
      Serial.printf("[Node] Sending: %s\n", json.c_str());
      sendMeshData(json);
    }
  }
}
