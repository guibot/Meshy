#pragma once
#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <map>
#include "config.h"

// ── Types ─────────────────────────────────────────────────────────────────────

struct NodeData {
  String        nodeId;
  String        location;
  String        mode          = "node";
  float         temperature   = NAN;
  float         humidity      = NAN;
  int           potentiometer = -1;
  bool          button        = false;
  bool          hasButton     = false;
  unsigned long lastSeenMs    = 0;
  int           hopCount      = -1;  // hops from root; -1 = unknown, 1 = direct
  uint32_t      parentId      = 0;   // direct parent node in mesh tree; 0 = root
  int           insertOrder   = 0;   // stable display position; assigned once on first seen
  String        rawJson;
};

// ── Globals ───────────────────────────────────────────────────────────────────
// No `static` on nodeRegistry — display.h and webserver.h include this header
// directly, so they share the same definition in the single translation unit.

static painlessMesh            _mesh;
std::map<uint32_t, NodeData>   nodeRegistry;   // root only
static int                     _nextOrder = 0;

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
  nd.mode          = doc["mode"] | "node";
  nd.lastSeenMs    = millis();
  nd.rawJson       = msg;

  JsonObjectConst s = doc["sensors"];
  if (s) {
    if (!s["temperature"].isNull())   nd.temperature   = s["temperature"].as<float>();
    if (!s["humidity"].isNull())      nd.humidity      = s["humidity"].as<float>();
    if (!s["potentiometer"].isNull()) nd.potentiometer = s["potentiometer"].as<int>();
    if (!s["button"].isNull())        { nd.hasButton = true; nd.button = s["button"].as<bool>(); }
  }

  // preserve hopCount and stable display order; assign insertOrder once on first seen
  auto existing = nodeRegistry.find(from);
  if (existing != nodeRegistry.end()) {
    nd.hopCount    = existing->second.hopCount;
    nd.parentId    = existing->second.parentId;
    nd.insertOrder = existing->second.insertOrder;
  } else {
    nd.insertOrder = _nextOrder++;
  }

  nodeRegistry[from] = nd;
}

// Recursively walk subConnectionJson tree, storing depth as hopCount per node
static void _parseHops(JsonObjectConst node, int depth, uint32_t parentId) {
  uint32_t id = node["nodeId"].as<uint32_t>();
  auto it = nodeRegistry.find(id);
  if (it != nodeRegistry.end()) {
    it->second.hopCount = depth;
    it->second.parentId = parentId;
  }
  JsonArrayConst subs = node["subs"].as<JsonArrayConst>();
  for (JsonObjectConst sub : subs) _parseHops(sub, depth + 1, id);
}

static void _onChanged() {
  int count = (int)_mesh.getNodeList().size();
  Serial.printf("[Mesh] Topology changed — %d node(s)\n", count);

  // Grace period: bump lastSeenMs for all online nodes so mesh reformation
  // doesn't trigger false offline while the network reconnects
  unsigned long now = millis();
  for (auto& kv : nodeRegistry) {
    if ((now - kv.second.lastSeenMs) < NODE_TIMEOUT)
      kv.second.lastSeenMs = now;
  }

  // Parse full topology tree — root is depth 0, direct nodes depth 1, etc.
  String topoJson = _mesh.subConnectionJson(true);
  Serial.printf("[Mesh] Topology: %s\n", topoJson.c_str());

  DynamicJsonDocument doc(2048);
  if (deserializeJson(doc, topoJson) == DeserializationError::Ok) {
    _parseHops(doc.as<JsonObjectConst>(), 0, 0);
  }
}
