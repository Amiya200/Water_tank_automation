// web_server.cpp
// ESP8266 <-> STM32 Web UART Bridge for Water Tank Controller
// Updated: All UART packets now follow the format table (e.g. @COMMAND:PARAM#)

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

#ifndef MOTOR_PIN
  #define MOTOR_PIN 2
#endif

// ===== STATE =====
static bool g_motorState = false;
static int g_waterLevel = 70;

static bool g_countdownActive = false;
static unsigned long g_countdownEnd = 0;
static bool g_countdownFinalMotor = false;

static String g_timerOn[5];
static String g_timerOff[5];

static String g_searchGap;
static String g_searchDry;

static String g_twistOnDur;
static String g_twistOffDur;


// ===== HELPERS =====
static void ws_setMotor(bool on) {
  g_motorState = on;
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, on ? HIGH : LOW);
  // Serial.println(on ? "⚙️ Motor: ON" : "⚙️ Motor: OFF");
}

static void ws_updateSimulatedWaterLevel() {
  if (g_motorState && g_countdownActive) {
    g_waterLevel += 1;
    if (g_waterLevel > 100) g_waterLevel = 100;
  } else if (!g_motorState) {
    g_waterLevel -= 1;
    if (g_waterLevel < 30) g_waterLevel = 30;
  }
}

static void ws_sendPacketToSTM32(const String &payload) {
  String pkt = payload;
  if (!pkt.endsWith("#")) pkt += "#";
  esp_uart_send(pkt.c_str());
}

static void ws_applyUartPacketToWeb(const char *raw) {
  if (!raw || !raw[0]) return;

  String pkt = String(raw);
  pkt.trim();
  if (pkt.endsWith("#")) pkt.remove(pkt.length() - 1);
  if (pkt.startsWith("@")) pkt.remove(0, 1);

  if (pkt == "MOTOR_ON") { ws_setMotor(true); return; }
  if (pkt == "MOTOR_OFF") { ws_setMotor(false); return; }
  if (pkt == "10W") { g_waterLevel = 10; return; }
  if (pkt == "30W") { g_waterLevel = 30; return; }
  if (pkt == "70W") { g_waterLevel = 70; return; }
  if (pkt == "1:W") { g_waterLevel = 100; return; }

  int p1 = pkt.indexOf(':');
  String cmd = (p1 == -1) ? pkt : pkt.substring(0, p1);
  String rest = (p1 == -1) ? "" : pkt.substring(p1 + 1);

  if (cmd == "MOTOR") {
    if (rest == "ON") ws_setMotor(true);
    if (rest == "OFF") ws_setMotor(false);
    return;
  }
}


// ===== HTTP ROUTES =====
void start_webserver() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  // ==== DASHBOARD ====
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  // ==== MANUAL MODE ====
  server.on("/manual", HTTP_GET, []() {
    server.send(200, "text/html", manualModeHtml);
  });
  server.on("/manual/on", HTTP_GET, []() {
    ws_setMotor(true);
    ws_sendPacketToSTM32("@MANUAL:ON#");
    server.send(200, "text/plain", "Manual Mode: Motor turned ON");
  });
  server.on("/manual/off", HTTP_GET, []() {
    ws_setMotor(false);
    ws_sendPacketToSTM32("@MANUAL:OFF#");
    server.send(200, "text/plain", "Manual Mode: Motor turned OFF");
  });

  // ==== COUNTDOWN MODE ====
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
    g_countdownEnd = millis() + (unsigned long)durationMin * 60UL * 1000UL;
    g_countdownActive = true;
    g_countdownFinalMotor = true;
    ws_sendPacketToSTM32("@COUNTDOWN:ON:" + String(durationMin) + "#");

    String response = "⏱ Motor ON for " + String(durationMin) +
                      " min. It will auto stop after countdown or full tank.";
    server.send(200, "text/plain", response);
  });
  server.on("/countdown_stop", HTTP_GET, []() {
    g_countdownActive = false;
    ws_setMotor(false);
    ws_sendPacketToSTM32("@COUNTDOWN:OFF#");
    server.send(200, "text/plain", "Countdown stopped manually.");
  });

  // ==== TIMER MODE ====
  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });
  server.on("/timer/set", HTTP_GET, []() {
    bool any = false;
    String resp = "Timer settings:\n";
    for (int i = 1; i <= 5; i++) {
      String on = server.arg("on" + String(i));
      String off = server.arg("off" + String(i));
      if (!on.isEmpty() && !off.isEmpty()) {
        any = true;
        g_timerOn[i - 1] = on;
        g_timerOff[i - 1] = off;
        int onH = on.substring(0, 2).toInt();
        int onM = on.substring(3, 5).toInt();
        int offH = off.substring(0, 2).toInt();
        int offM = off.substring(3, 5).toInt();
        ws_sendPacketToSTM32("@TIMER:SET:" + String(onH) + ":" + String(onM) + ":" + String(offH) + ":" + String(offM) + "#");
        delay(3);
        resp += "Slot " + String(i) + ": " + on + "→" + off + "\n";
      }
    }
    if (any)
      server.send(200, "text/plain", resp);
    else
      server.send(400, "text/plain", "No valid timer slots.");
  });
  server.on("/timer_stop", HTTP_GET, []() {
    ws_sendPacketToSTM32("@TIMER:STOP#");
    server.send(200, "text/plain", "Timer mode stopped.");
  });

  // ==== SEARCH MODE ====
  server.on("/search", HTTP_GET, []() {
    server.send(200, "text/html", searchModeHtml);
  });
  server.on("/search_submit", HTTP_GET, []() {
    g_searchGap = server.arg("gap");
    g_searchDry = server.arg("dryrun");
    ws_sendPacketToSTM32("@SEARCH:SET:" + g_searchGap + ":" + g_searchDry + "#");
    String resp = "Search mode: GAP=" + g_searchGap + "s, PROBE=" + g_searchDry + "s";
    server.send(200, "text/plain", resp);
  });
  server.on("/search_stop", HTTP_GET, []() {
    ws_sendPacketToSTM32("@SEARCH:STOP#");
    server.send(200, "text/plain", "Search mode stopped.");
  });

  // ==== TWIST MODE ====
  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });
  server.on("/twist_submit", HTTP_GET, []() {
    g_twistOnDur = server.arg("onDuration");
    g_twistOffDur = server.arg("offDuration");
    ws_sendPacketToSTM32("@TWIST:SET:" + g_twistOnDur + ":" + g_twistOffDur + "#");
    server.send(200, "text/plain", "Twist cycle set.");
  });
  server.on("/twist_stop", HTTP_GET, []() {
    ws_sendPacketToSTM32("@TWIST:STOP#");
    server.send(200, "text/plain", "Twist mode stopped.");
  });

  // ==== SEMI AUTO ====
  server.on("/semi", HTTP_GET, []() {
    server.send(200, "text/html", semiAutoModeHtml);
  });
  server.on("/semi_toggle", HTTP_GET, []() {
    g_motorState = !g_motorState;
    ws_setMotor(g_motorState);
    ws_sendPacketToSTM32(g_motorState ? "@SEMIAUTO:ON#" : "@SEMIAUTO:OFF#");
    server.send(200, "text/plain", g_motorState ? "ON" : "OFF");
  });

  // ==== STATUS ====
server.on("/status_request", HTTP_GET, []() {
  ws_sendPacketToSTM32("@STATUS#");
  server.send(200, "text/plain", "Status requested from STM32.");
});


  // ==== PING ====
  server.on("/ping", HTTP_GET, []() {
    ws_sendPacketToSTM32("@PING#");
    server.send(200, "text/plain", "Ping sent to STM32.");
  });

  server.begin();
  Serial.println("✅ HTTP server started");
}


// ===== LOOP HANDLER =====
static void ws_handleCountdownLogic() {
  if (!g_countdownActive) return;

  if (g_waterLevel >= 100) {
    Serial.println("💧 Tank Full! Stopping early.");
    g_countdownActive = false;
    ws_setMotor(false);
    ws_sendPacketToSTM32("@COUNTDOWN:DONE:OFF#");
    return;
  }

  if (millis() > g_countdownEnd) {
    g_countdownActive = false;
    ws_setMotor(false);
    ws_sendPacketToSTM32("@COUNTDOWN:DONE:OFF#");
    Serial.println("⏰ Countdown finished → Motor OFF");
  }
}

void handleClient() {
  server.handleClient();
  ws_handleCountdownLogic();

  char rxBuf[128];
  if (esp_uart_receive(rxBuf, sizeof(rxBuf))) {
    Serial.print("UART RX -> ");
    Serial.println(rxBuf);
    ws_applyUartPacketToWeb(rxBuf);
  }
}
