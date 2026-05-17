# meshy_espcode

Distributed sensor mesh network for agricultural/industrial monitoring, built on [painlessMesh](https://gitlab.com/painlessMesh/painlessMesh).

## Hardware

| Role | Board | Notes |
|---|---|---|
| Root gateway | M5Stack Core2 | Touch display, web dashboard, WiFi uplink |
| Sensor node | ESP32-C3 Super Mini | Potentiometer, push button, AHT10 (optional) |

## Features

- Self-healing mesh — nodes auto-route through each other if out of direct range
- Root display shows all nodes: location, sensor values, hop count, last-seen time
- Multi-hop relay name shown for nodes not directly connected to root
- Offline detection — stale nodes sorted to bottom and flagged OFFLINE
- Web dashboard at root IP (`/` — live table, `/data` — JSON)
- WiFi + mesh coexist on root (painlessMesh AP+STA mode)

## Build

```bash
# Node build (ESP32-C3)
arduino-cli compile --fqbn esp32:esp32:esp32c3 meshy_espcode/

# Root build (M5Stack Core2)
arduino-cli compile --fqbn m5stack:esp32:m5stack_core2 meshy_espcode/
```

Flash:
```bash
arduino-cli upload -p <PORT> --fqbn <FQBN> meshy_espcode/
# List ports: arduino-cli board list
```

## Configuration

Edit `config.h` before flashing:

- `NODE_LOCATION` — unique label per device (shown on display and dashboard)
- `MESH_SSID` / `MESH_PASSWORD` — shared across all nodes
- `WIFI_SSID` / `WIFI_PASSWORD` — external WiFi for root
- `ENABLE_DISPLAY` + `ENABLE_WEBSERVER` — uncomment both for root build; leave commented for node build
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
