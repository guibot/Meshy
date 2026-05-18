# TODO

## Node parameter control (from root)
- Settings page on root lists nodes with a slider per node (0.5–5s broadcast interval)
- Root sends `{"cmd":"set_interval","ms":2000}` via `_mesh.sendSingle(nodeId, msg)`
- Node `_onReceive` handles `cmd` payloads and updates broadcast interval
- Optional: persist to ESP32 NVS (`Preferences`) so reboot survives; otherwise root re-sends on topology change
