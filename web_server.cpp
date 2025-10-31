// web_server.cpp
// ESP8266 web <-> UART bridge for water-tank controller

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

// dashboard HTML is in a .cpp
extern const char *htmlContent;

#ifndef MOTOR_PIN
  #define MOTOR_PIN 2
#endif

// ===== STATE =====
static bool g_motorState = false;
static int g_waterLevel = 70;

// countdown
static bool g_countdownActive = false;
static unsigned long g_countdownEnd = 0;
static bool g_countdownFinalMotor = false;

// timer
static String g_timerOn[5];
static String g_timerOff[5];

// search
static String g_searchGap;
static String g_searchDry;
static String g_searchDays;
static String g_searchOnTime;
static String g_searchOffTime;

// twist
static String g_twistOnDur;
static String g_twistOffDur;
static String g_twistOnTime;
static String g_twistOffTime;
static String g_twistDays;


// ===== HELPERS =====
static void ws_setMotor(bool on) {
  g_motorState = on;
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, on ? HIGH : LOW);
  Serial.println(on ? "⚙️ Motor: ON" : "⚙️ Motor: OFF");
}

static void ws_updateSimulatedWaterLevel() {
  // simulate simple behavior for testing
  if (g_motorState && g_countdownActive) {
    g_waterLevel += 1;
    if (g_waterLevel > 100) g_waterLevel = 100;
  } else if (!g_motorState) {
    g_waterLevel -= 1;
    if (g_waterLevel < 30) g_waterLevel = 30;
  }
}

static void ws_sendPacketToSTM32(const String &payload) {
  esp_uart_send(payload.c_str()); // auto '#' + newline
}

static void ws_applyUartPacketToWeb(const char *raw) {
  if (!raw || !raw[0]) return;

  String pkt = String(raw);
  pkt.trim();

  if (pkt.endsWith("#")) pkt.remove(pkt.length() - 1);
  if (pkt.startsWith("@")) pkt.remove(0, 1);

  // backward compatible motor
  if (pkt == "MOTOR_ON") { ws_setMotor(true); return; }
  if (pkt == "MOTOR_OFF") { ws_setMotor(false); return; }

  // backward compatible water
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
    ws_sendPacketToSTM32("@MANUAL:ON");
    server.send(200, "text/plain", "Manual Mode: Motor turned ON");
  });
  server.on("/manual/off", HTTP_GET, []() {
    ws_setMotor(false);
    ws_sendPacketToSTM32("@MANUAL:OFF");
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

    // ✅ Turn motor ON immediately
    ws_setMotor(true);
    g_countdownEnd = millis() + (unsigned long)durationMin * 60UL * 1000UL;
    g_countdownActive = true;
    g_countdownFinalMotor = true;  // OFF at end
    ws_sendPacketToSTM32("@COUNTDOWN:ON:" + String(durationMin));

    String response = "⏱ Motor turned ON instantly for " + String(durationMin) +
                      " minute(s). It will turn OFF automatically after countdown or when tank is full.";
    server.send(200, "text/plain", response);
  });


  // ==== TIMER MODE ====
  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });
  server.on("/timer/set", HTTP_GET, []() {
    ws_sendPacketToSTM32("@TIMER:CLEAR");
    delay(5);

    bool any = false;
    String resp = "Timer settings received:\n";

    for (int i = 1; i <= 5; i++) {
      String on = server.arg("on" + String(i));
      String off = server.arg("off" + String(i));
      if (!on.length()) on = server.arg("onTime" + String(i));
      if (!off.length()) off = server.arg("offTime" + String(i));

      if (on.length() && off.length()) {
        any = true;
        g_timerOn[i - 1] = on;
        g_timerOff[i - 1] = off;
        ws_sendPacketToSTM32("@TIMER:SET:" + String(i) + ":" + on + ":" + off);
        delay(3);
        resp += "Slot " + String(i) + ": " + on + " -> " + off + "\n";
      }
    }

    if (any)
      server.send(200, "text/plain", resp);
    else {
      String dbg = "No valid timer slots received.\nGot args:\n";
      for (int i = 0; i < server.args(); i++)
        dbg += server.argName(i) + " = " + server.arg(i) + "\n";
      server.send(400, "text/plain", dbg);
    }
  });


  // ==== SEARCH MODE ====
  server.on("/search", HTTP_GET, []() {
    server.send(200, "text/html", searchModeHtml);
  });
  server.on("/search_submit", HTTP_GET, []() {
    String gap = server.arg("gap");
    String dry = server.arg("dryrun");
    String onTime = server.arg("onTime");
    String offTime = server.arg("offTime");

    String days;
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "days") {
        if (days.length()) days += " ";
        days += server.arg(i);
      }
    }

    g_searchGap = gap;
    g_searchDry = dry;
    g_searchOnTime = onTime;
    g_searchOffTime = offTime;
    g_searchDays = days;

    ws_sendPacketToSTM32("@SEARCH:SET:" + gap + ":" + dry + ":" + onTime + ":" + offTime + ":" + days);

    String resp = "Search settings saved:\n";
    resp += "Gap: " + gap + "\nDryRun: " + dry + "\nON Time: " + onTime +
            "\nOFF Time: " + offTime + "\nDays: " + days + "\n";
    server.send(200, "text/plain", resp);
  });


  // ==== TWIST MODE ====
  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });
  server.on("/twist_submit", HTTP_GET, []() {
    g_twistOnDur = server.arg("onDuration");
    g_twistOffDur = server.arg("offDuration");
    g_twistOnTime = server.arg("onTime");
    g_twistOffTime = server.arg("offTime");

    String days;
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "days") {
        if (days.length()) days += " ";
        days += server.arg(i);
      }
    }
    g_twistDays = days;

    ws_sendPacketToSTM32("@TWIST:SET:" + g_twistOnDur + ":" + g_twistOffDur + ":" +
                         g_twistOnTime + ":" + g_twistOffTime + ":" + g_twistDays);
    server.send(200, "text/plain", "Twist settings saved");
  });


  // ==== SEMI AUTO ====
  server.on("/semi", HTTP_GET, []() {
    server.send(200, "text/html", semiAutoModeHtml);
  });
  server.on("/semi_toggle", HTTP_GET, []() {
    g_motorState = !g_motorState;
    ws_setMotor(g_motorState);
    ws_sendPacketToSTM32(g_motorState ? "@SEMI:ON" : "@SEMI:OFF");
    server.send(200, "text/plain", g_motorState ? "ON" : "OFF");
  });


  // ==== MOTOR STATUS ====
  server.on("/motor_status", HTTP_GET, []() {
    server.send(200, "text/plain", g_motorState ? "ON" : "OFF");
  });

  // ==== WATER LEVEL STATUS ====
  server.on("/status", HTTP_GET, []() {
    ws_updateSimulatedWaterLevel();
    server.send(200, "application/json",
                "{\"level\":" + String(g_waterLevel) + "}");
  });

  // ==== DEBUG STATE JSON ====
  server.on("/state.json", HTTP_GET, []() {
    String json = "{";
    json += "\"motor\":" + String(g_motorState ? "true" : "false") + ",";
    json += "\"level\":" + String(g_waterLevel);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("✅ HTTP server started");
}


// ===== LOOP HOOK =====
static void ws_handleCountdownLogic() {
  if (!g_countdownActive) return;

  // stop early if water full
  if (g_waterLevel >= 100) {
    Serial.println("💧 Tank Full! Motor stopped early.");
    g_countdownActive = false;
    ws_setMotor(false);
    ws_sendPacketToSTM32("@COUNTDOWN:DONE:OFF");
    return;
  }

  // normal timeout expiry
  if (millis() > g_countdownEnd) {
    g_countdownActive = false;
    ws_setMotor(false);
    ws_sendPacketToSTM32("@COUNTDOWN:DONE:OFF");
    Serial.println("⏰ Countdown finished → Motor OFF");
  }
}


void handleClient() {
  server.handleClient();
  ws_handleCountdownLogic();

  // poll UART continuously
  char rxBuf[128];
  if (esp_uart_receive(rxBuf, sizeof(rxBuf))) {
    Serial.print("UART RX -> ");
    Serial.println(rxBuf);
    ws_applyUartPacketToWeb(rxBuf);
  }
}
