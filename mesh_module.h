#pragma once
#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <map>
#include "config.h"

// ── Types ─────────────────────────────────────────────────────────────────────

struct NodeData {
  String        nodeId;
  String        location;
  float         temperature   = NAN;
  float         humidity      = NAN;
  int           potentiometer = -1;
  bool          button        = false;
  unsigned long lastSeenMs    = 0;   // millis() on root at time of receipt
  String        rawJson;
};

// ── Globals ───────────────────────────────────────────────────────────────────
// No `static` on nodeRegistry — display.h and webserver.h include this header
// directly, so they share the same definition in the single translation unit.

static painlessMesh            _mesh;
std::map<uint32_t, NodeData>   nodeRegistry;  // root only

// ── Forward declarations ──────────────────────────────────────────────────────

static void _onReceive(uint32_t from, String& msg);
static void _onChanged();

// ── Public API ────────────────────────────────────────────────────────────────

void setupMesh(bool isRoot) {
  _mesh.setDebugMsgTypes(ERROR | STARTUP);
  _mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
  if (isRoot) {
    _mesh.setRoot(true);
  }
  _mesh.setContainsRoot(true);
  _mesh.onReceive(&_onReceive);
  _mesh.onChangedConnections(&_onChanged);
  Serial.printf("[Mesh] Initialised as %s | nodeId: %u\n",
    isRoot ? "ROOT" : "NODE", _mesh.getNodeId());
}

void updateMesh() {
  _mesh.update();
}

void sendMeshData(const String& json) {
  _mesh.sendBroadcast(json);
  Serial.printf("[Mesh] Broadcast sent (%d bytes)\n", json.length());
}

uint32_t getMeshNodeId() {
  return _mesh.getNodeId();
}

int getMeshNodeCount() {
  return (int)_mesh.getNodeList().size();
}

const std::map<uint32_t, NodeData>& getNodeRegistry() {
  return nodeRegistry;
}

// ── Private callbacks ─────────────────────────────────────────────────────────

static void _onReceive(uint32_t from, String& msg) {
  Serial.printf("[Mesh] Rx from %u: %s\n", from, msg.c_str());

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, msg) != DeserializationError::Ok) {
    Serial.println("[Mesh] JSON parse failed — discarding");
    return;
  }

  NodeData nd;
  nd.nodeId        = doc["node_id"] | String(from, HEX);
  nd.location      = doc["location"] | "unknown";
  nd.lastSeenMs    = millis();
  nd.rawJson       = msg;

  JsonObjectConst s = doc["sensors"];
  if (s) {
    if (!s["temperature"].isNull())   nd.temperature   = s["temperature"].as<float>();
    if (!s["humidity"].isNull())      nd.humidity      = s["humidity"].as<float>();
    if (!s["potentiometer"].isNull()) nd.potentiometer = s["potentiometer"].as<int>();
    if (!s["button"].isNull())        nd.button        = s["button"].as<bool>();
  }

  nodeRegistry[from] = nd;
}

static void _onChanged() {
  Serial.printf("[Mesh] Topology changed — %d node(s)\n",
    (int)_mesh.getNodeList().size());
}
