#pragma once
#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_WEBSERVER

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "mesh_module.h"  // provides NodeData, nodeRegistry

static AsyncWebServer _webServer(80);

// ── HTML dashboard (stored in flash) ─────────────────────────────────────────

static const char _DASHBOARD_HTML[] PROGMEM = R"html(
<!DOCTYPE html><html><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>meshy</title>
<style>
*{box-sizing:border-box}
body{font-family:monospace;background:#111;color:#eee;padding:1em;margin:0}
h2{color:#4af;margin:0 0 .5em 0}
.node{border:1px solid #2a2a2a;padding:.8em;margin:.4em 0;border-radius:6px;background:#1a1a1a}
.loc{font-weight:bold;font-size:1.1em}
.age{color:#555;font-size:.8em;float:right}
.vals{margin-top:.4em;color:#4f4;line-height:1.8}
.btn-on{color:#ff4}
.btn-off{color:#555}
.empty{color:#555;padding:1em}
</style></head><body>
<h2>&#9679; meshy mesh</h2>
<div id="root"></div>
<script>
function refresh(){
  fetch('/data').then(r=>r.json()).then(nodes=>{
    var el=document.getElementById('root');
    if(!nodes.length){el.innerHTML='<p class="empty">No nodes connected yet.</p>';return;}
    el.innerHTML=nodes.map(function(n){
      var s=n.sensors;
      var vals='';
      if(s.temperature!==undefined) vals+='&#127777; '+s.temperature.toFixed(1)+'&deg;C &nbsp;';
      if(s.humidity!==undefined)    vals+='&#128167; '+s.humidity.toFixed(0)+'% &nbsp;';
      if(s.potentiometer!==undefined) vals+='&#127908; '+s.potentiometer+' &nbsp;';
      if(s.button!==undefined)      vals+='<span class="'+(s.button?'btn-on':'btn-off')+'">&#9679; btn:'+(s.button?'ON':'OFF')+'</span>';
      return '<div class="node">'
        +'<span class="loc">'+n.location+'</span>'
        +'<span class="age">'+n.last_seen_s+'s ago &mdash; '+n.node_id+'</span>'
        +'<div class="vals">'+vals+'</div>'
        +'</div>';
    }).join('');
  }).catch(function(){
    document.getElementById('root').innerHTML='<p class="empty">Error fetching data.</p>';
  });
}
refresh();
setInterval(refresh,5000);
</script></body></html>
)html";

// ── /data JSON ────────────────────────────────────────────────────────────────

static String _buildDataJson() {
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.to<JsonArray>();

  for (auto& kv : nodeRegistry) {
    const NodeData& nd = kv.second;
    JsonObject node = arr.createNestedObject();
    node["node_id"]     = nd.nodeId;
    node["location"]    = nd.location;
    node["last_seen_s"] = (millis() - nd.lastSeenMs) / 1000UL;

    JsonObject sensors = node.createNestedObject("sensors");
    if (!isnan(nd.temperature))   sensors["temperature"]   = nd.temperature;
    if (!isnan(nd.humidity))      sensors["humidity"]      = nd.humidity;
    if (nd.potentiometer >= 0)    sensors["potentiometer"] = nd.potentiometer;
    sensors["button"] = nd.button;
  }

  String out;
  serializeJson(doc, out);
  return out;
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setupWebServer() {
  _webServer.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send_P(200, "text/html", _DASHBOARD_HTML);
  });

  _webServer.on("/data", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(200, "application/json", _buildDataJson());
  });

  _webServer.onNotFound([](AsyncWebServerRequest* req) {
    req->send(404, "text/plain", "not found");
  });

  _webServer.begin();
  Serial.println("[Web] Server started on port 80");
}

#else  // no-op for node build

void setupWebServer() {}

#endif  // ENABLE_WEBSERVER
