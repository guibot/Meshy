# meshy_espcode

Distributed sensor mesh network for agricultural/industrial monitoring, built on [painlessMesh](https://gitlab.com/painlessMesh/painlessMesh).

## Hardware

| Role | Board |
|---|---|
| Root gateway | M5Stack Core2 |
| Sensor node | M5Stack Atom, ESP32-C3 Super Mini, XIAO ESP32-C3, XIAO ESP32-S3 |

## Features

- Self-healing mesh — nodes auto-route through each other if out of direct range
- Root display shows all nodes: location, sensor values, hop count, last-seen
- Per-node status circle: green (active), blue (idle), grey (offline)
- Offline detection — stale nodes sorted to bottom and flagged OFFLINE
- Web dashboard at root IP (`/` — live table, `/data` — JSON)
- WiFi + mesh coexist on root (painlessMesh AP+STA mode)

## Build (Arduino IDE)

1. Open `meshy_espcode.ino` in Arduino IDE
2. Select the correct board under **Tools → Board**
3. Edit `config.h` (see below) then upload

## Configuration

Edit `config.h` before flashing:

- `IS_ROOT` — uncomment for root build (M5Stack Core2); leave commented for nodes
- `BOARD_xxx` — uncomment the board being flashed (one at a time)
- `NODE_LOCATION` — unique label per device (shown on display and dashboard)
- `MESH_SSID` / `MESH_PASSWORD` — shared across all devices
- `WIFI_SSID` / `WIFI_PASSWORD` — external WiFi for root only
- `SEND_INTERVAL` — how often nodes broadcast sensor data (ms)

## Module Map

| File | Purpose |
|---|---|
| `config.h` | All tunable parameters |
| `mesh_module.h` | painlessMesh init, node registry, topology parsing |
| `sensors.h` | AHT10, potentiometer, button (ifdef-gated) |
| `display.h` | M5Core2 touch display (no-op stub for node build) |
| `webserver.h` | ESPAsyncWebServer dashboard (no-op stub for node build) |
| `mqtt_stub.h` | MQTT no-op stub (phase 2) |

## Phase 2

Replace `mqtt_stub.h` body with PubSubClient. Interface (`setupMQTT()`, `publishMQTT(topic, payload)`) stays identical. Call `publishMQTT()` from `_onReceive()` in `mesh_module.h`.
