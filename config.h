#pragma once

// ── Hardware profile ──────────────────────────────────────────────────────────
// Uncomment for ROOT build (M5Stack Core2); leave commented for NODE build
//#define IS_ROOT

#ifdef IS_ROOT
  #define ENABLE_DISPLAY
  #define ENABLE_WEBSERVER
#endif

// ── Board selection (uncomment one) ──────────────────────────────────────────
// #define BOARD_ESP32C3_SUPERMINI
#define BOARD_M5ATOM
// #define BOARD_XIAO_ESP32C3
// #define BOARD_XIAO_ESP32S3


// ── Node identity (change per device) ────────────────────────────────────────
#define NODE_LOCATION   "02_ATOM" // "04_AHT10" // "01_BT_NEO" // "03_BT_POT"

// ── Sensor enable flags (node build only) ────────────────────────────────────
#ifndef IS_ROOT 
#define ENABLE_AHT10
//#define ENABLE_POTENTIOMETER
#define ENABLE_BUTTON
#endif

#if defined(BOARD_ESP32C3_SUPERMINI)
  #define ANALOG_PIN       4
  #define BUTTON_PIN       3
  #define PIN_SDA          8
  #define PIN_SCL          9

#elif defined(BOARD_M5ATOM)
  #define ANALOG_PIN       22
  #define BUTTON_PIN       39
  #define PIN_SDA          25
  #define PIN_SCL          21
  #define NEOPIXEL         27

#elif defined(BOARD_XIAO_ESP32C3)
  #define ANALOG_PIN       2
  #define BUTTON_PIN       3
  #define PIN_SDA          4
  #define PIN_SCL          5

#elif defined(BOARD_XIAO_ESP32S3)
  #define ANALOG_PIN       2
  #define BUTTON_PIN       3
  #define PIN_SDA          4
  #define PIN_SCL          5

#elif defined(IS_ROOT)
  // root (M5Stack Core2) needs no pin definitions
#else
  #error "No board selected — uncomment one BOARD_xxx in config.h"
#endif


// ── Mesh ──────────────────────────────────────────────────────────────────────
#define MESH_SSID       "meshy_net"
#define MESH_PASSWORD   "meshy_pass"
#define MESH_PORT       5555

// ── Timing ────────────────────────────────────────────────────────────────────
#define SEND_INTERVAL     5000   // ms between sensor broadcasts (node)
#define DISPLAY_INTERVAL   1000   // ms between display refresh (root)
#define NODE_TIMEOUT      (SEND_INTERVAL * 3)  // ms without data → node marked offline


// ── WiFi (root only) ──────────────────────────────────────────────────────────
#include "wifi_secrets.h"  // copy wifi_secrets_template.h → wifi_secrets.h and fill in credentials

// ── MQTT — phase 2 (stub only for now) ───────────────────────────────────────
#define MQTT_BROKER     "192.168.1.100"
#define MQTT_PORT       1883
#define MQTT_USER       ""
#define MQTT_PASSWORD   ""
