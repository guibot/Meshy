#include <Preferences.h>
#include "config.h"
#include "mesh_module.h"
#include "mqtt_stub.h"
#include "sensors.h"
#include "display.h"
#include "webserver.h"

// ── Globals ───────────────────────────────────────────────────────────────────

Preferences prefs;
bool        isRoot = false;
String      wifiIP = "";

// ── Mode detection ────────────────────────────────────────────────────────────

void detectAndSaveMode() {
#ifndef ENABLE_DISPLAY
  // On plain ESP32, GPIO 0 is the BOOT button
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  delay(100);
  bool buttonHeld = (digitalRead(MODE_BUTTON_PIN) == LOW);
  prefs.begin("meshy", false);
  if (buttonHeld) {
    isRoot = true;
    prefs.putBool("isRoot", true);
    Serial.println("[Mode] Button held — saving ROOT mode to NVS");
  } else {
    isRoot = prefs.getBool("isRoot", false);
  }
  prefs.end();
#else
  // On M5Core2, always ROOT (display build = root build)
  isRoot = true;
#endif
  Serial.printf("[Mode] Running as: %s\n", isRoot ? "ROOT" : "NODE");
}

// ── WiFi (root only) ──────────────────────────────────────────────────────────

#ifdef ENABLE_WEBSERVER
#include <WiFi.h>

void setupWiFi() {
  // painlessMesh runs WiFi in AP+STA mode. On root, after mesh.init() sets up the
  // mesh AP, we connect the STA interface to external WiFi. mesh.setRoot(true)
  // prevents painlessMesh from using STA to seek other roots.
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
  delay(200);
  Serial.println("\n[Boot] meshy_espcode starting...");

  detectAndSaveMode();
  setupMesh(isRoot);

  if (isRoot) {
    setupWiFi();
    setupDisplay();
    setupWebServer();
    setupMQTT();
  } else {
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
