#pragma once

// ── Hardware profile ──────────────────────────────────────────────────────────
// Uncomment both for ROOT build (M5Stack Core2); leave commented for NODE build
 #define ENABLE_DISPLAY     // M5Unified touch display
 #define ENABLE_WEBSERVER   // ESPAsyncWebServer

// ── Mesh ──────────────────────────────────────────────────────────────────────
#define MESH_SSID       "meshy_net"
#define MESH_PASSWORD   "meshy_pass"
#define MESH_PORT       5555

// ── Node identity (change per device) ────────────────────────────────────────
#define NODE_LOCATION   "02_BT_Pot"

// ── Timing ────────────────────────────────────────────────────────────────────
#define SEND_INTERVAL     5000   // ms between sensor broadcasts (node)
#define DISPLAY_INTERVAL   1000   // ms between display refresh (root)
#define NODE_TIMEOUT      (SEND_INTERVAL * 3)  // ms without data → node marked offline

// ── Pins (node) ───────────────────────────────────────────────────────────────
#define MODE_BUTTON_PIN   0    // hold LOW on boot → ROOT mode this session
#define POT_PIN           4    // analog potentiometer — ESP32-C3 ADC pins: 0-4 only
#define BUTTON_PIN       10    // digital push button (active LOW)

// ── Sensor enable flags (node build only) ────────────────────────────────────
// Auto-disabled for root build when ENABLE_DISPLAY is defined
#ifndef ENABLE_DISPLAY
// #define ENABLE_AHT10
#define ENABLE_POTENTIOMETER
#define ENABLE_BUTTON
#endif

// ── WiFi (root only) ──────────────────────────────────────────────────────────
#include "wifi_secrets.h"  // copy wifi_secrets_template.h → wifi_secrets.h and fill in credentials

// ── MQTT — phase 2 (stub only for now) ───────────────────────────────────────
#define MQTT_BROKER     "192.168.1.100"
#define MQTT_PORT       1883
#define MQTT_USER       ""
#define MQTT_PASSWORD   ""
