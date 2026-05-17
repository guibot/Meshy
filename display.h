#pragma once
#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_DISPLAY

#include <M5Unified.h>
#include "mesh_module.h"  // provides NodeData, nodeRegistry, getMeshNodeCount()

// ── Layout ────────────────────────────────────────────────────────────────────

static const int _TOPBAR_H = 30;
static const int _ROW_H    = 52;
static const int _MAX_ROWS = 4;   // (240 - 30) / 52 = 4 rows

static int           _scrollOffset  = 0;
static unsigned long _lastDisplayMs = 0;

// ── Helpers ───────────────────────────────────────────────────────────────────

static String _fmtElapsed(unsigned long lastSeenMs) {
  unsigned long s = (millis() - lastSeenMs) / 1000;
  if (s < 60)   return String(s) + "s ago";
  if (s < 3600) return String(s / 60) + "m ago";
  return String(s / 3600) + "h ago";
}

static void _drawTopBar(const String& ip) {
  M5.Display.fillRect(0, 0, 320, _TOPBAR_H, (uint16_t)0x0019);  // dark navy
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(4, 10);
  M5.Display.printf("meshy  nodes:%d  %s", getMeshNodeCount(), ip.c_str());
}

static void _drawNodeRow(int y, const NodeData& nd) {
  M5.Display.fillRect(0, y, 320, _ROW_H - 1, (uint16_t)0x2104);  // dark grey
  M5.Display.drawFastHLine(0, y + _ROW_H - 1, 320, TFT_DARKGREY);

  M5.Display.setTextSize(1);

  // Line 1: location + elapsed
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(4, y + 5);
  M5.Display.printf("%-18s", nd.location.c_str());
  M5.Display.setTextColor(TFT_DARKGREY);
  M5.Display.print(_fmtElapsed(nd.lastSeenMs));

  // Line 2: sensor values
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.setCursor(4, y + 22);
  if (!isnan(nd.temperature))  M5.Display.printf("%.1fC ", nd.temperature);
  if (!isnan(nd.humidity))     M5.Display.printf("%.0f%% ", nd.humidity);
  if (nd.potentiometer >= 0)   M5.Display.printf("pot:%d ", nd.potentiometer);
  M5.Display.setTextColor(nd.button ? TFT_YELLOW : TFT_DARKGREY);
  M5.Display.printf("btn:%s", nd.button ? "ON" : "OFF");

  // Line 3: node id
  M5.Display.setTextColor((uint16_t)0x4208);  // dim grey
  M5.Display.setCursor(4, y + 38);
  M5.Display.print(nd.nodeId);
}

// ── Public API ────────────────────────────────────────────────────────────────

void setupDisplay() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);         // landscape
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextFont(1);
  Serial.println("[Display] M5Core2 display ready (320x240)");
}

void updateDisplay(const String& ip) {
  if (millis() - _lastDisplayMs < DISPLAY_INTERVAL) return;
  _lastDisplayMs = millis();

  M5.update();   // process touch / button events

  // Touch scroll
  if (M5.Touch.getCount() > 0) {
    auto detail = M5.Touch.getDetail();
    if (detail.wasFlicked()) {
      int dy = detail.distanceY();
      if (dy < -40) _scrollOffset++;
      if (dy >  40) _scrollOffset--;
    }
  }

  int total = (int)nodeRegistry.size();
  _scrollOffset = constrain(_scrollOffset, 0, max(0, total - _MAX_ROWS));

  _drawTopBar(ip);

  int row = 0, idx = 0;
  for (auto& kv : nodeRegistry) {
    if (idx++ < _scrollOffset) continue;
    if (row >= _MAX_ROWS) break;
    _drawNodeRow(_TOPBAR_H + row * _ROW_H, kv.second);
    row++;
  }

  // Clear unused rows
  for (int r = row; r < _MAX_ROWS; r++) {
    M5.Display.fillRect(0, _TOPBAR_H + r * _ROW_H, 320, _ROW_H, TFT_BLACK);
  }
}

#else  // ENABLE_DISPLAY not defined — no-op stubs for node build

void setupDisplay() {}
void updateDisplay(const String&) {}

#endif  // ENABLE_DISPLAY
