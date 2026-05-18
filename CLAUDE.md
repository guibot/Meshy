# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

painlessMesh sensor network for agricultural/industrial monitoring.
Single Arduino sketch (`.ino` + per-concern `.h` headers).
Hardware: M5Stack Core2 (root gateway) + ESP32 devkits (sensor nodes).

## Build Targets

| Target | Board | config.h flags |
|---|---|---|
| Root | M5Stack Core2 | `#define IS_ROOT` |
| Node | M5Atom / ESP32-C3 SuperMini / XIAO C3 / XIAO S3 | `IS_ROOT` commented out, `BOARD_xxx` uncommented |

## Build

Arduino IDE. Select board under Tools → Board, edit `config.h`, upload.

## Module Responsibilities

- `config.h` — all tunable parameters; change `NODE_LOCATION` per device
- `mesh_module.h` — painlessMesh; exposes `nodeRegistry` (root only)
- `sensors.h` — AHT10, potentiometer, button; guarded by `ENABLE_*` defines
- `display.h` — M5Core2 display; entire file is a no-op if `ENABLE_DISPLAY` not set
- `webserver.h` — AsyncWebServer `/` + `/data`; no-op if `ENABLE_WEBSERVER` not set
- `mqtt_stub.h` — no-op stub for phase 2

## Key Constraints

- No `delay()` in `loop()` — all timing via `millis()` deltas
- `updateMesh()` must be called every loop — it drives the painlessMesh scheduler
- `display.h` and `webserver.h` include `mesh_module.h` directly — no manual include ordering required
- painlessMesh manages WiFi in AP+STA mode; root's STA connects to external WiFi after `mesh.init()`
- No deep sleep — nodes stay awake to maintain mesh topology

## Phase 2 (MQTT)

Replace `mqtt_stub.h` body with PubSubClient implementation.
Interface (`setupMQTT()`, `publishMQTT(topic, payload)`) stays identical.
Call `publishMQTT()` from `_onReceive()` in `mesh_module.h`.
