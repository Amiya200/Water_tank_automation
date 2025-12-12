// web_server.cpp
// ESP8266 <-> STM32 Web UART Bridge for Water Tank Controller
// Updated for new Timer Mode: Enable/Disable + Days + ON/OFF
// Also adds Settings page support (GET/SET + parsing of STATUS:...:SETTINGS:...#)

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WebServer.h>
  static ESP8266WebServer server(80);
#else
  #include <WebServer.h>
  static WebServer server(80);
#endif

#include "web_server.h"
#include "countdown_mode.h"
#include "timer_mode.h"
#include "twist_mode.h"
#include "auto_mode.h"
#include "esp_uart_comm.h"
#include "settings_mode.h"   // new: settings HTML

extern const char *htmlContent;

// ===== GLOBAL STATE =====
String g_motorStatus = "OFF";
int g_liveLevel = 0;
String g_liveMode = "UNKNOWN";

#ifndef MOTOR_PIN
#define MOTOR_PIN 2
#endif

// ===== INTERNAL STATE =====
static bool g_motorState = false;
static int g_waterLevel = 0;

static bool g_countdownActive = false;
static unsigned long g_countdownEnd = 0;
static bool g_countdownFinalMotor = false;

static unsigned long g_lastStatusReq = 0;

// ===== Settings cache (exposed to Settings page) =====
struct TimerSlot {
  int enabled;
  String on;
  String off;
};

struct SettingsCache {
  String deviceName = "MyDevice";
  String opMode = "AUTO";
  int defaultLevel = 5;
  int twistOn = 1;
  int twistOnDur = 30;
  int twistOffDur = 10;
  int sensorThreshold = 45;
  float currentAlarm = 5.0;
  float voltageAlarm = 200.0;
  String relay1Label = "Pump";
  String relay2Label = "Valve";
  String relay3Label = "Light";
  TimerSlot timers[5];
  String rawExtras = "";
} g_settings;

// ===== HELPERS =====
static void ws_setMotor(bool on) {
  g_motorState = on;
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, on ? HIGH : LOW);
  g_motorStatus = on ? "ON" : "OFF";
}

static void ws_sendPacketToSTM32(const String &payload) {
  String pkt = payload;
  if (!pkt.endsWith("#")) pkt += "#";
  esp_uart_send(pkt.c_str());
}

static void ws_requestStatus() {
  ws_sendPacketToSTM32("@STATUS#");
  g_lastStatusReq = millis();
}

// URL-decode helper (basic: handles %20 and +)
static String urlDecode(const String &str) {
  String ret;
  char c;
  for (size_t i = 0; i < str.length(); i++) {
    c = str[i];
    if (c == '+') {
      ret += ' ';
    } else if (c == '%' && i + 2 < str.length()) {
      char hi = str[i + 1];
      char lo = str[i + 2];
      char hexBuf[3] = { hi, lo, 0 };
      char decoded = (char) strtol(hexBuf, NULL, 16);
      ret += decoded;
      i += 2;
    } else {
      ret += c;
    }
  }
  return ret;
}

// parse timers value "idx:enabled,on,off|idx2:..."
static void parseTimersField(const String &val) {
  if (val.length() == 0) return;
  int start = 0;
  while (start < val.length()) {
    int bar = val.indexOf('|', start);
    String piece;
    if (bar == -1) { piece = val.substring(start); start = val.length(); }
    else { piece = val.substring(start, bar); start = bar + 1; }
    int colon = piece.indexOf(':');
    if (colon <= 0) continue;
    int idx = piece.substring(0, colon).toInt();
    if (idx < 0 || idx >= 5) continue;
    String rest = piece.substring(colon + 1); // enabled,on,off
    int c1 = rest.indexOf(',');
    int c2 = rest.indexOf(',', c1 + 1);
    if (c1 == -1 || c2 == -1) continue;
    g_settings.timers[idx].enabled = rest.substring(0, c1).toInt();
    g_settings.timers[idx].on = rest.substring(c1 + 1, c2);
    g_settings.timers[idx].off = rest.substring(c2 + 1);
  }
}

// parse key=value;key2=val;... and update g_settings
static void parseSettingsKV(const String &kvStr) {
  if (kvStr.length() == 0) return;
  int start = 0;
  while (start < kvStr.length()) {
    int semi = kvStr.indexOf(';', start);
    String token;
    if (semi == -1) { token = kvStr.substring(start); start = kvStr.length(); }
    else { token = kvStr.substring(start, semi); start = semi + 1; }
    token.trim();
    if (token.length() == 0) continue;
    int eq = token.indexOf('=');
    if (eq == -1) continue;
    String k = token.substring(0, eq);
    String v = token.substring(eq + 1);
    k.trim(); v.trim();
    // decode values that may be urlEncoded
    String dec = urlDecode(v);
    if (k == "deviceName") g_settings.deviceName = dec;
    else if (k == "opMode") g_settings.opMode = dec;
    else if (k == "defaultLevel") g_settings.defaultLevel = dec.toInt();
    else if (k == "twistOn") g_settings.twistOn = dec.toInt();
    else if (k == "twistOnDur") g_settings.twistOnDur = dec.toInt();
    else if (k == "twistOffDur") g_settings.twistOffDur = dec.toInt();
    else if (k == "sensorThreshold") g_settings.sensorThreshold = dec.toInt();
    else if (k == "currentAlarm") g_settings.currentAlarm = dec.toFloat();
    else if (k == "voltageAlarm") g_settings.voltageAlarm = dec.toFloat();
    else if (k == "relay1Label") g_settings.relay1Label = dec;
    else if (k == "relay2Label") g_settings.relay2Label = dec;
    else if (k == "relay3Label") g_settings.relay3Label = dec;
    else if (k == "timers") parseTimersField(dec);
    else {
      // unknown key, append to rawExtras
      if (g_settings.rawExtras.length()) g_settings.rawExtras += ";";
      g_settings.rawExtras += k + "=" + dec;
    }
  }
}

// ======================================================
// APPLY UART PACKET → WEB GLOBALS + AUTO MODE
// ======================================================
static void ws_applyUartPacketToWeb(const char *raw) {
  if (!raw || !raw[0]) return;

  String pkt = String(raw);
  pkt.trim();

  if (pkt.startsWith("@")) pkt.remove(0, 1);
  if (pkt.endsWith("#")) pkt.remove(pkt.length() - 1);

  // STATUS PACKET
  if (pkt.startsWith("STATUS:")) {
    // we will look for optional ":SETTINGS:" chunk
    int settingsPos = pkt.indexOf(":SETTINGS:");
    String settingsPart = "";
    String main = pkt;
    if (settingsPos != -1) {
      settingsPart = pkt.substring(settingsPos + 10); // after ":SETTINGS:"
      main = pkt.substring(0, settingsPos);
    }

    main.replace("STATUS:", "");

    int m1 = main.indexOf("MOTOR:");
    int l1 = main.indexOf(":LEVEL:");
    int mo1 = main.indexOf(":MODE:");

    if (m1 != -1 && l1 != -1 && mo1 != -1) {
      g_motorStatus = main.substring(m1 + 6, l1);

      int levelCode = main.substring(l1 + 7, mo1).toInt();
      switch (levelCode) {
        case 1: g_liveLevel = 20; break;
        case 2: g_liveLevel = 40; break;
        case 3: g_liveLevel = 60; break;
        case 4: g_liveLevel = 80; break;
        case 5: g_liveLevel = 100; break;
        default: g_liveLevel = 0; break;
      }

      g_liveMode = main.substring(mo1 + 6);

      autoMode_applyStatusFromSTM32(g_liveMode);

      // parse settings if present
      if (settingsPart.length()) {
        // settingsPart is "k1=v1;k2=v2;..."
        parseSettingsKV(settingsPart);
        // optionally: notify UI / websockets here if implemented
      }

      Serial.printf("STATUS → Motor:%s | Level:%d | Mode:%s\n",
        g_motorStatus.c_str(), g_liveLevel, g_liveMode.c_str());
    }
    return;
  }

  // LEGACY
  if (pkt == "MOTOR_ON") { g_motorState = true; return; }
  if (pkt == "MOTOR_OFF") { g_motorState = false; return; }

  // Water Levels
  if (pkt == "10W") g_waterLevel = 10;
  if (pkt == "30W") g_waterLevel = 30;
  if (pkt == "70W") g_waterLevel = 70;
  if (pkt == "1:W") g_waterLevel = 100;

  int p = pkt.indexOf(':');
  String cmd = (p == -1 ? pkt : pkt.substring(0, p));
  String rest = (p == -1 ? "" : pkt.substring(p + 1));

  if (cmd == "MOTOR") {
    if (rest == "ON") ws_setMotor(true);
    if (rest == "OFF") ws_setMotor(false);
  }
}

// ======================================================
// START WEB SERVER
// ======================================================
void start_webserver() {

  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  g_lastStatusReq = millis();

  // MAIN DASHBOARD
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  // JSON STATUS
  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"motor\":\"" + g_motorStatus + "\",";
    json += "\"level\":" + String(g_liveLevel) + ",";
    json += "\"mode\":\"" + g_liveMode + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  // SETTINGS PAGE (HTML)
  server.on("/settings", HTTP_GET, []() {
    server.send(200, "text/html", settingsModeHtml);
  });

  // SETTINGS GET -> return g_settings as JSON
  server.on("/settings/get", HTTP_GET, []() {
    String json = "{";
    json += "\"deviceName\":\"" + g_settings.deviceName + "\",";
    json += "\"opMode\":\"" + g_settings.opMode + "\",";
    json += "\"defaultLevel\":" + String(g_settings.defaultLevel) + ",";
    json += "\"twistOn\":" + String(g_settings.twistOn) + ",";
    json += "\"twistOnDur\":" + String(g_settings.twistOnDur) + ",";
    json += "\"twistOffDur\":" + String(g_settings.twistOffDur) + ",";
    json += "\"sensorThreshold\":" + String(g_settings.sensorThreshold) + ",";
    json += "\"currentAlarm\":" + String(g_settings.currentAlarm) + ",";
    json += "\"voltageAlarm\":" + String(g_settings.voltageAlarm) + ",";
    json += "\"relay1Label\":\"" + g_settings.relay1Label + "\",";
    json += "\"relay2Label\":\"" + g_settings.relay2Label + "\",";
    json += "\"relay3Label\":\"" + g_settings.relay3Label + "\",";
    json += "\"rawExtras\":\"" + g_settings.rawExtras + "\",";
    json += "\"timers\":[";
    for (int i = 0; i < 5; i++) {
      if (i) json += ",";
      json += "{";
      json += "\"enabled\":" + String(g_settings.timers[i].enabled) + ",";
      json += "\"on\":\"" + g_settings.timers[i].on + "\",";
      json += "\"off\":\"" + g_settings.timers[i].off + "\"";
      json += "}";
    }
    json += "]}";
    server.send(200, "application/json", json);
  });

  // SETTINGS SET -> receives "data" query param containing key=value;key2=val...
  server.on("/settings/set", HTTP_GET, []() {
    if (!server.hasArg("data")) {
      server.send(400, "text/plain", "Missing data");
      return;
    }
    String data = server.arg("data");
    // Build packet for STM32 and send
    String packet = "@SETTINGS:" + data;
    if (!packet.endsWith("#")) packet += "#";
    ws_sendPacketToSTM32(packet);

    // wait for ACK (short window)
    const unsigned long TIMEOUT_MS = 600;
    char rxBuf[256];
    String resp = "";
    unsigned long start = millis();
    bool got = false;
    while (millis() - start < TIMEOUT_MS) {
      if (esp_uart_receive(rxBuf, sizeof(rxBuf), 10)) {
        resp = String(rxBuf);
        got = true;
        break;
      }
      delay(1);
    }

    // ask for fresh status which should include SETTINGS if STM32 sends them
    ws_requestStatus();

    if (got) {
      server.send(200, "text/plain", resp);
    } else {
      server.send(200, "text/plain", "No ACK from STM32 (timeout). Status refresh requested.");
    }
  });

  // AUTO MODE
  server.on("/auto", HTTP_GET, []() {
    server.send(200, "text/html", autoModeHtml);
  });

  server.on("/auto_toggle", HTTP_GET, []() {
    autoMode_toggle();
    ws_requestStatus();
    server.send(200, "text/plain", autoMode_getStatusText());
  });

  // MANUAL
  server.on("/manual_toggle", HTTP_GET, []() {
    if (g_liveMode == "MANUAL") {
      ws_sendPacketToSTM32("@MANUAL:OFF#");
      g_liveMode = "OFFMODE";
    } else {
      ws_sendPacketToSTM32("@MANUAL:ON#");
      g_liveMode = "MANUAL";
    }
    ws_requestStatus();
    server.send(200, "text/plain", "OK");
  });

  // SEMI AUTO
  server.on("/semi_toggle", HTTP_GET, []() {
    if (g_liveMode == "SEMIAUTO") {
      ws_sendPacketToSTM32("@SEMIAUTO:OFF#");
      g_liveMode = "OFFMODE";
    } else {
      ws_sendPacketToSTM32("@SEMIAUTO:ON#");
      g_liveMode = "SEMIAUTO";
    }
    ws_requestStatus();
    server.send(200, "text/plain", "OK");
  });

  // COUNTDOWN MODE
  server.on("/countdown", HTTP_GET, []() {
    server.send(200, "text/html", countdownModeHtml);
  });

  server.on("/start_countdown", HTTP_GET, []() {
    if (!server.hasArg("duration")) {
      server.send(400, "text/plain", "Missing duration");
      return;
    }

    int durationMin = server.arg("duration").toInt();
    if (durationMin <= 0) {
      server.send(400, "text/plain", "Invalid duration");
      return;
    }

    ws_setMotor(true);
    g_countdownActive = true;
    g_countdownEnd = millis() + (unsigned long)durationMin * 60000UL;

    ws_sendPacketToSTM32("@COUNTDOWN:ON:" + String(durationMin) + "#");
    ws_requestStatus();
    server.send(200, "text/plain", "Countdown started.");
  });

  // STOP COUNTDOWN
  server.on("/countdown_stop", HTTP_GET, []() {
    g_countdownActive = false;
    ws_setMotor(false);
    ws_sendPacketToSTM32("@COUNTDOWN:OFF#");
    ws_requestStatus();
    server.send(200, "text/plain", "Countdown stopped.");
  });

  // ------------------------------------------------------
  // TIMER PAGE (HTML)
  // ------------------------------------------------------
  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });

  // ------------------------------------------------------
  // TIMER SET ROUTE — ENABLE + DAYS + ON/OFF SUPPORT
  // ------------------------------------------------------
  server.on("/timer/set", HTTP_GET, []() {

    String packet = "@TIMER:SET";
    bool any = false;
    String summary = "";

    for (int i = 1; i <= 5; i++) {

      if (!server.hasArg("slot" + String(i))) continue;
      any = true;

      String days = server.arg("days" + String(i));
      String on   = server.arg("on"   + String(i));
      String off  = server.arg("off"  + String(i));

      int onH  = on.substring(0,2).toInt();
      int onM  = on.substring(3,5).toInt();
      int offH = off.substring(0,2).toInt();
      int offM = off.substring(3,5).toInt();

      packet += ":" + String(i) + ":" + days + ":" +
                String(onH) + ":" + String(onM) + ":" +
                String(offH) + ":" + String(offM);

      summary += "• Slot " + String(i) +
        " | Days: " + days +
        " | " + on + " → " + off + "\n";
    }

    if (!any) {
      server.send(400, "text/plain", "No valid slot selected");
      return;
    }

    packet += "#";
    ws_sendPacketToSTM32(packet);

    char rxBuf[128];
    String resp = "";
    unsigned long start = millis();

    while (millis() - start < 400) {
      if (esp_uart_receive(rxBuf, sizeof(rxBuf), 10)) {
        resp = rxBuf;
        break;
      }
      delay(1);
    }

    ws_requestStatus();

    String answer = "TIMER UPDATED SUCCESSFULLY:\n\n";
    answer += summary;
    answer += "\nSTM32 Response: " + resp;

    server.send(200, "text/plain", answer);
  });

  // TWIST MODE
  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });

  server.on("/twist_submit", HTTP_GET, []() {
    String onDur = server.arg("onDuration");
    String offDur = server.arg("offDuration");
    String onTime = server.arg("onTime");
    String offTime = server.arg("offTime");

    String days = "";
    const char *allDays[] = {"mon","tue","wed","thu","fri","sat","sun"};

    for (auto d : allDays) {
      if (server.hasArg(d)) {
        if (days.length() > 0) days += ",";
        days += d;
      }
    }

    String packet = "@TWIST:SET:" + onDur + ":" + offDur + ":" +
                    onTime + ":" + offTime + ":" + days + "#";
    ws_sendPacketToSTM32(packet);

    ws_requestStatus();
    server.send(200, "text/plain", "TWIST settings applied");
  });

  server.begin();
  Serial.println("Web server started");
}

// ======================================================
// LOOP HANDLER
// ======================================================
static void ws_handleCountdownLogic() {
  if (!g_countdownActive) return;

  if (g_waterLevel >= 100 || millis() > g_countdownEnd) {
    g_countdownActive = false;
    ws_setMotor(false);
    ws_sendPacketToSTM32("@COUNTDOWN:DONE:OFF#");
    ws_requestStatus();
    Serial.println("Countdown done → Motor OFF");
  }
}

void handleClient() {
  server.handleClient();
  ws_handleCountdownLogic();

  char rxBuf[256];
  if (esp_uart_receive(rxBuf, sizeof(rxBuf), 10)) {
    Serial.print("UART RX → ");
    Serial.println(rxBuf);
    ws_applyUartPacketToWeb(rxBuf);
  }
}
