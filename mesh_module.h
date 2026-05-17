#pragma once
#include <Arduino.h>

struct NodeData {
  String nodeId;
  String location;
  float  temperature   = NAN;
  float  humidity      = NAN;
  int    potentiometer = -1;
  bool   button        = false;
  unsigned long lastSeenMs = 0;
};

void setupMesh(bool isRoot) {}
void updateMesh() {}
void sendMeshData(const String& json) {}
uint32_t getMeshNodeId() { return 0; }
int getMeshNodeCount() { return 0; }
