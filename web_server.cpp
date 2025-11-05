// web_server.cpp
// ESP8266 <-> STM32 Web UART Bridge for Water Tank Controller
// Updated: live water level, motor, and mode updates + dynamic status intervals
// Added: level mapping (1→20, 2→40, 3→60, 4→80, 5→100)

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
#include "manual_mode.h"
#include "search_mode.h"
#include "semi_auto_mode.h"
#include "timer_mode.h"
#include "twist_mode.h"
#include "esp_uart_comm.h"

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

static String g_searchGap;
static String g_searchDry;
static String g_twistOnDur;
static String g_twistOffDur;

static unsigned long g_lastStatusReq = 0;

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
  // Serial.println("📡 Requested @STATUS#");
}

// dynamic 5 s / 5 min update
static void ws_autoStatusRefresh() {
  // unsigned long now = millis();
  // unsigned long interval = (g_motorStatus == "ON") ? 5000UL : 5UL * 60UL * 1000UL;
  // if (now - g_lastStatusReq >= interval) {
  //   ws_requestStatus();
  //   g_lastStatusReq = now;
  // }
}

// apply STM32 packet to web globals
static void ws_applyUartPacketToWeb(const char *raw) {
  if (!raw || !raw[0]) return;
  String pkt = String(raw);
  pkt.trim();
  if (pkt.startsWith("@")) pkt.remove(0, 1);
  if (pkt.endsWith("#")) pkt.remove(pkt.length() - 1);

  if (pkt.startsWith("STATUS:")) {
    // Example: @STATUS:MOTOR:ON:LEVEL:3:MODE:MANUAL#
    pkt.replace("STATUS:", "");
    int m1 = pkt.indexOf("MOTOR:");
    int l1 = pkt.indexOf(":LEVEL:");
    int mo1 = pkt.indexOf(":MODE:");
    if (m1 != -1 && l1 != -1 && mo1 != -1) {
      g_motorStatus = pkt.substring(m1 + 6, l1);

      // ---- NEW MAPPING ----
      int levelCode = pkt.substring(l1 + 7, mo1).toInt();
      switch (levelCode) {
        case 1: g_liveLevel = 20; break;
        case 2: g_liveLevel = 40; break;
        case 3: g_liveLevel = 60; break;
        case 4: g_liveLevel = 80; break;
        case 5: g_liveLevel = 100; break;
        default: g_liveLevel = 0; break;
      }

      g_liveMode = pkt.substring(mo1 + 6);
      Serial.printf("STATUS → Motor:%s | Level:%d | Mode:%s\n",
                    g_motorStatus.c_str(), g_liveLevel, g_liveMode.c_str());
    }
    return;
  }

  // legacy compatibility
  if (pkt == "MOTOR_ON") {
    g_motorState = true;
    return;
  }
  if (pkt == "MOTOR_OFF") {
    g_motorState = false;
    return;
  }
  if (pkt == "10W") {
    g_waterLevel = 10;
    return;
  }
  if (pkt == "30W") {
    g_waterLevel = 30;
    return;
  }
  if (pkt == "70W") {
    g_waterLevel = 70;
    return;
  }
  if (pkt == "1:W") {
    g_waterLevel = 100;
    return;
  }

  int p1 = pkt.indexOf(':');
  String cmd = (p1 == -1) ? pkt : pkt.substring(0, p1);
  String rest = (p1 == -1) ? "" : pkt.substring(p1 + 1);
  if (cmd == "MOTOR") {
    if (rest == "ON") ws_setMotor(true);
    if (rest == "OFF") ws_setMotor(false);
  }
}

// ===== WEB ROUTES =====
void start_webserver() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  g_lastStatusReq = millis();

  // --- DASHBOARD ---
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  // --- LIVE STATUS JSON ---
  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"motor\":\"" + g_motorStatus + "\",";
    json += "\"level\":" + String(g_liveLevel) + ",";
    json += "\"mode\":\"" + g_liveMode + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  // --- MANUAL MODE ---
  server.on("/manual", HTTP_GET, []() {
    server.send(200, "text/html", manualModeHtml);
  });
  server.on("/manual/on", HTTP_GET, []() {
    ws_setMotor(true);
    ws_sendPacketToSTM32("@MANUAL:ON#");
    ws_requestStatus();
    server.send(200, "text/plain", "Motor ON");
  });
  server.on("/manual/off", HTTP_GET, []() {
    ws_setMotor(false);
    ws_sendPacketToSTM32("@MANUAL:OFF#");
    ws_requestStatus();
    server.send(200, "text/plain", "Motor OFF");
  });

  // --- COUNTDOWN MODE ---
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
    g_countdownEnd = millis() + (unsigned long)durationMin * 60UL * 1000UL;
    ws_sendPacketToSTM32("@COUNTDOWN:ON:" + String(durationMin) + "#");
    ws_requestStatus();
    server.send(200, "text/plain", "Countdown started.");
  });
  server.on("/countdown_stop", HTTP_GET, []() {
    g_countdownActive = false;
    ws_setMotor(false);
    ws_sendPacketToSTM32("@COUNTDOWN:OFF#");
    ws_requestStatus();
    server.send(200, "text/plain", "Countdown stopped.");
  });

  // --- TIMER MODE ---
  // === TIMER PAGE ===
  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });

  // === TIMER SET (all slots) ===
    // === TIMER SET (all slots) ===
  server.on("/timer/set", HTTP_GET, []() {
    String packet = "@TIMER:SET";
    bool any = false;
    String humanSummary = "";

    for (int i = 1; i <= 5; i++) {
      if (!server.hasArg("on" + String(i)) || !server.hasArg("off" + String(i))) continue;
      String on = server.arg("on" + String(i));
      String off = server.arg("off" + String(i));
      if (on.length() < 5 || off.length() < 5) continue;

      any = true;
      int onH = on.substring(0, 2).toInt();
      int onM = on.substring(3, 5).toInt();
      int offH = off.substring(0, 2).toInt();
      int offM = off.substring(3, 5).toInt();
      packet += ":" + String(onH) + ":" + String(onM) + ":" + String(offH) + ":" + String(offM);

      humanSummary += "• Slot " + String(i) + ": " + on + " → " + off + "\n";
    }

    if (!any) {
      server.send(400, "text/plain", "⚠️ No valid ON/OFF slot filled.");
      return;
    }

    packet += "#";
    ws_sendPacketToSTM32(packet);

    // Wait briefly for ACK
    char rxBuf[128];
    String resp = "";
    unsigned long start = millis();
    while (millis() - start < 400) {
      if (esp_uart_receive(rxBuf, sizeof(rxBuf))) {
        resp = rxBuf;
        break;
      }
      delay(1);
    }

    ws_requestStatus();

    String responseText = "✅ Timer slots updated successfully!\n\n";
    responseText += humanSummary;
    // responseText += "\nRaw Packet → " + packet + "\n";

    if (resp.startsWith("@ACK") || resp.indexOf("TIMER_OK") >= 0)
      responseText += "📩 STM32 ACK: " + resp;
    else
      responseText += "ℹ️ No ACK received from STM32";

    server.send(200, "text/plain", responseText);
  });


  // === TIMER STOP ===
  server.on("/timer_stop", HTTP_GET, []() {
    ws_sendPacketToSTM32("@TIMER:STOP#");

    // optional: short wait for ACK
    char rxBuf[128];
    String resp = "";
    unsigned long start = millis();
    while (millis() - start < 300) {
      if (esp_uart_receive(rxBuf, sizeof(rxBuf))) {
        resp = rxBuf;
        break;
      }
      delay(1);
    }

    ws_requestStatus();
    server.send(200, "text/plain", resp.isEmpty() ? "Timer stopped (no ACK)" : resp);
  });


  // --- SEARCH MODE ---
  server.on("/search", HTTP_GET, []() {
    server.send(200, "text/html", searchModeHtml);
  });
  server.on("/search_submit", HTTP_GET, []() {
    g_searchGap = server.arg("gap");
    g_searchDry = server.arg("dryrun");
    ws_sendPacketToSTM32("@SEARCH:SET:" + g_searchGap + ":" + g_searchDry + "#");
    ws_requestStatus();
    server.send(200, "text/plain", "Search updated.");
  });
  server.on("/search_stop", HTTP_GET, []() {
    ws_sendPacketToSTM32("@SEARCH:STOP#");
    ws_requestStatus();
    server.send(200, "text/plain", "Search stopped.");
  });

  // --- TWIST MODE ---
  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });
  server.on("/twist_submit", HTTP_GET, []() {
    g_twistOnDur = server.arg("onDuration");
    g_twistOffDur = server.arg("offDuration");
    ws_sendPacketToSTM32("@TWIST:SET:" + g_twistOnDur + ":" + g_twistOffDur + "#");
    ws_requestStatus();
    server.send(200, "text/plain", "Twist set.");
  });
  server.on("/twist_stop", HTTP_GET, []() {
    ws_sendPacketToSTM32("@TWIST:STOP#");
    ws_requestStatus();
    server.send(200, "text/plain", "Twist stopped.");
  });

  // --- SEMI AUTO MODE ---
  server.on("/semi", HTTP_GET, []() {
    server.send(200, "text/html", semiAutoModeHtml);
  });
  server.on("/semi_toggle", HTTP_GET, []() {
    g_motorState = !g_motorState;
    ws_setMotor(g_motorState);
    ws_sendPacketToSTM32(g_motorState ? "@SEMIAUTO:ON#" : "@SEMIAUTO:OFF#");
    ws_requestStatus();
    server.send(200, "text/plain", g_motorState ? "ON" : "OFF");
  });

  server.begin();
  Serial.println("✅ Web server started");
}

// ===== LOOP HANDLER =====
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
  // ws_autoStatusRefresh();

  char rxBuf[128];
  if (esp_uart_receive(rxBuf, sizeof(rxBuf))) {
    Serial.print("UART RX → ");
    Serial.println(rxBuf);
    ws_applyUartPacketToWeb(rxBuf);
  }
}
