#pragma once
#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_DISPLAY

#include <M5Unified.h>
#include <vector>
#include <algorithm>
#include "mesh_module.h"  // provides NodeData, nodeRegistry, getMeshNodeCount()

// ── Layout ────────────────────────────────────────────────────────────────────

static const int _TOPBAR_H = 30;
static const int _ROW_H    = 52;
static const int _MAX_ROWS = 4;

static int           _scrollOffset  = 0;
static unsigned long _lastDisplayMs = 0;

// ── Helpers ───────────────────────────────────────────────────────────────────

static bool _isOnline(const NodeData& nd) {
  return (millis() - nd.lastSeenMs) < NODE_TIMEOUT;
}

static String _fmtElapsed(unsigned long lastSeenMs) {
  unsigned long s = (millis() - lastSeenMs) / 1000;
  if (s < 60)   return String(s) + "s ago";
  if (s < 3600) return String(s / 60) + "m ago";
  return String(s / 3600) + "h ago";
}

static void _drawTopBar(const String& ip) {
  M5.Display.fillRect(0, 0, 320, _TOPBAR_H, (uint16_t)0x0019);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(4, 10);
  M5.Display.printf("meshy  nodes:%d  %s", getMeshNodeCount(), ip.c_str());
}

static void _drawNodeRow(int y, const NodeData& nd, bool online) {
  uint16_t bgColor   = online ? (uint16_t)0x2104 : (uint16_t)0x1800;  // dark grey : dark red-grey
  uint16_t dimColor  = online ? TFT_DARKGREY      : (uint16_t)0x4208;
  uint16_t nameColor = online ? TFT_WHITE          : (uint16_t)0x8410;  // white : mid grey

  M5.Display.fillRect(0, y, 320, _ROW_H - 1, bgColor);
  M5.Display.drawFastHLine(0, y + _ROW_H - 1, 320, dimColor);
  M5.Display.setTextSize(1);

  // Line 1: location + elapsed
  M5.Display.setTextColor(nameColor);
  M5.Display.setCursor(4, y + 5);
  M5.Display.printf("%-18s", nd.location.c_str());
  M5.Display.setTextColor(dimColor);
  M5.Display.print(_fmtElapsed(nd.lastSeenMs));

  // Line 2: sensor values or OFFLINE label
  M5.Display.setCursor(4, y + 22);
  if (online) {
    M5.Display.setTextColor(TFT_GREEN);
    if (!isnan(nd.temperature))  M5.Display.printf("%.1fC ", nd.temperature);
    if (!isnan(nd.humidity))     M5.Display.printf("%.0f%% ", nd.humidity);
    if (nd.potentiometer >= 0)   M5.Display.printf("pot:%d ", nd.potentiometer);
    M5.Display.setTextColor(nd.button ? TFT_YELLOW : TFT_DARKGREY);
    M5.Display.printf("btn:%s", nd.button ? "ON" : "OFF");
  } else {
    M5.Display.setTextColor((uint16_t)0x6B4D);  // muted orange-red
    M5.Display.print("OFFLINE");
  }

  // Line 3: node id
  M5.Display.setTextColor(dimColor);
  M5.Display.setCursor(4, y + 38);
  M5.Display.print(nd.nodeId);
}

// ── Public API ────────────────────────────────────────────────────────────────

void setupDisplay() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextFont(1);
  Serial.println("[Display] M5Core2 display ready (320x240)");
}

void updateDisplay(const String& ip) {
  if (millis() - _lastDisplayMs < DISPLAY_INTERVAL) return;
  _lastDisplayMs = millis();

  M5.update();

  // Touch scroll
  if (M5.Touch.getCount() > 0) {
    auto detail = M5.Touch.getDetail();
    if (detail.wasFlicked()) {
      int dy = detail.distanceY();
      if (dy < -40) _scrollOffset++;
      if (dy >  40) _scrollOffset--;
    }
  }

  // Build sorted list: online nodes first, offline nodes last
  // Within each group, most recently seen first
  std::vector<const NodeData*> sorted;
  sorted.reserve(nodeRegistry.size());
  for (auto& kv : nodeRegistry) sorted.push_back(&kv.second);

  std::sort(sorted.begin(), sorted.end(), [](const NodeData* a, const NodeData* b) {
    bool aOn = _isOnline(*a);
    bool bOn = _isOnline(*b);
    if (aOn != bOn) return aOn;  // online before offline
    return a->lastSeenMs > b->lastSeenMs;  // newer first within group
  });

  int total = (int)sorted.size();
  _scrollOffset = constrain(_scrollOffset, 0, max(0, total - _MAX_ROWS));

  _drawTopBar(ip);

  int row = 0;
  for (int i = _scrollOffset; i < total && row < _MAX_ROWS; i++, row++) {
    _drawNodeRow(_TOPBAR_H + row * _ROW_H, *sorted[i], _isOnline(*sorted[i]));
  }

  // Clear unused rows
  for (int r = row; r < _MAX_ROWS; r++) {
    M5.Display.fillRect(0, _TOPBAR_H + r * _ROW_H, 320, _ROW_H, TFT_BLACK);
  }
}

#else  // no-op stubs for node build

void setupDisplay() {}
void updateDisplay(const String&) {}

#endif  // ENABLE_DISPLAY
