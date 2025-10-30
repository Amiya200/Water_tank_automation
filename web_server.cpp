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
extern const char* htmlContent;

#ifndef MOTOR_PIN
  #define MOTOR_PIN 2
#endif

// ===== STATE =====
static bool   g_motorState          = false;
static int    g_waterLevel          = 70;

// countdown
static bool   g_countdownActive     = false;
static unsigned long g_countdownEnd = 0;
static bool   g_countdownFinalMotor = false;

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
}

static void ws_updateSimulatedWaterLevel() {
  g_waterLevel = random(30, 100);
}

static void ws_sendPacketToSTM32(const String &payload) {
  // payload must start with '@'
  esp_uart_send(payload.c_str());  // adds '#'
}

// parse UART packet -> update UI state
static void ws_applyUartPacketToWeb(const char *raw) {
  if (!raw || !raw[0]) return;

  String pkt = String(raw);
  pkt.trim();

  if (pkt.endsWith("#")) pkt.remove(pkt.length() - 1);
  if (pkt.startsWith("@")) pkt.remove(0, 1);

  // backward compatible
  if (pkt == "MOTOR_ON")  { ws_setMotor(true);  return; }
  if (pkt == "MOTOR_OFF") { ws_setMotor(false); return; }

  // backward compatible water
  if (pkt == "10W") { g_waterLevel = 10;  return; }
  if (pkt == "30W") { g_waterLevel = 30;  return; }
  if (pkt == "70W") { g_waterLevel = 70;  return; }
  if (pkt == "1:W") { g_waterLevel = 100; return; }

  // new format
  int p1 = pkt.indexOf(':');
  String cmd  = (p1 == -1) ? pkt : pkt.substring(0, p1);
  String rest = (p1 == -1) ? ""  : pkt.substring(p1 + 1);

  // MOTOR:ON / MOTOR:OFF
  if (cmd == "MOTOR") {
    if (rest == "ON")  ws_setMotor(true);
    if (rest == "OFF") ws_setMotor(false);
    return;
  }

  // TIMER:...
  if (cmd == "TIMER") {
    if (rest == "CLEAR") {
      for (int i = 0; i < 5; i++) {
        g_timerOn[i]  = "";
        g_timerOff[i] = "";
      }
      return;
    }

    // TIMER:SET:<slot>:<on>:<off>
    if (rest.startsWith("SET:")) {
      int p2 = rest.indexOf(':', 4);
      int p3 = (p2 == -1) ? -1 : rest.indexOf(':', p2 + 1);
      if (p2 != -1 && p3 != -1) {
        int slot = rest.substring(4, p2).toInt();
        String on  = rest.substring(p2 + 1, p3);
        String off = rest.substring(p3 + 1);
        if (slot >= 1 && slot <= 5) {
          g_timerOn[slot - 1]  = on;
          g_timerOff[slot - 1] = off;
        }
      }
    }
    return;
  }

  // SEARCH:SET:<gap>:<dry>:<onTime>:<offTime>:<days>
  if (cmd == "SEARCH") {
    if (rest.startsWith("SET:")) {
      String body = rest.substring(4);
      int a = body.indexOf(':');
      int b = (a == -1) ? -1 : body.indexOf(':', a + 1);
      int c = (b == -1) ? -1 : body.indexOf(':', b + 1);
      int d = (c == -1) ? -1 : body.indexOf(':', c + 1);

      if (a != -1 && b != -1 && c != -1 && d != -1) {
        g_searchGap     = body.substring(0, a);
        g_searchDry     = body.substring(a + 1, b);
        g_searchOnTime  = body.substring(b + 1, c);
        g_searchOffTime = body.substring(c + 1, d);
        g_searchDays    = body.substring(d + 1);
      } else if (a != -1 && b != -1) {
        // backward compatibility
        g_searchGap  = body.substring(0, a);
        g_searchDry  = body.substring(a + 1, b);
        g_searchDays = body.substring(b + 1);
      }
    }
    return;
  }

  // TWIST:SET:...
  if (cmd == "TWIST") {
    if (rest.startsWith("SET:")) {
      String body = rest.substring(4);
      int a = body.indexOf(':');
      int b = (a == -1) ? -1 : body.indexOf(':', a + 1);
      int c = (b == -1) ? -1 : body.indexOf(':', b + 1);
      int d = (c == -1) ? -1 : body.indexOf(':', c + 1);
      if (a != -1 && b != -1 && c != -1 && d != -1) {
        g_twistOnDur   = body.substring(0, a);
        g_twistOffDur  = body.substring(a + 1, b);
        g_twistOnTime  = body.substring(b + 1, c);
        g_twistOffTime = body.substring(c + 1, d);
        g_twistDays    = body.substring(d + 1);
      }
    }
    return;
  }
}


// ===== HTTP ROUTES =====
void start_webserver() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  // dashboard
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  // MANUAL
  server.on("/manual", HTTP_GET, []() {
    server.send(200, "text/html", manualModeHtml);
  });
  server.on("/manual/on", HTTP_GET, []() {
    ws_setMotor(true);
    ws_sendPacketToSTM32("@MOTOR:ON");
    server.send(200, "text/plain", "Motor turned ON");
  });
  server.on("/manual/off", HTTP_GET, []() {
    ws_setMotor(false);
    ws_sendPacketToSTM32("@MOTOR:OFF");
    server.send(200, "text/plain", "Motor turned OFF");
  });

  // COUNTDOWN
  server.on("/countdown", HTTP_GET, []() {
    server.send(200, "text/html", countdownModeHtml);
  });
  server.on("/start_countdown", HTTP_GET, []() {
    if (!server.hasArg("duration") || !server.hasArg("mode")) {
      server.send(400, "text/plain", "Missing duration or mode");
      return;
    }
    int durationMin = server.arg("duration").toInt();
    String mode = server.arg("mode"); // "on" or "off"

    g_countdownEnd      = millis() + (unsigned long)durationMin * 60UL * 1000UL;
    g_countdownActive   = true;

    if (mode == "on") {
      ws_setMotor(true);
      g_countdownFinalMotor = false;
      ws_sendPacketToSTM32("@COUNTDOWN:ON:" + String(durationMin));
      server.send(200, "text/plain", "Motor ON for " + String(durationMin) + " min, then OFF");
    } else {
      ws_setMotor(false);
      g_countdownFinalMotor = true;
      ws_sendPacketToSTM32("@COUNTDOWN:OFF:" + String(durationMin));
      server.send(200, "text/plain", "Motor OFF for " + String(durationMin) + " min, then ON");
    }
  });

  // TIMER
  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });

  server.on("/timer/set", HTTP_GET, []() {
    ws_sendPacketToSTM32("@TIMER:CLEAR");
    delay(5);

    bool any = false;
    String resp = "Timer settings received:\n";

    for (int i = 1; i <= 5; i++) {
      String on  = server.arg("on"  + String(i));
      String off = server.arg("off" + String(i));
      if (!on.length())
        on = server.arg("onTime" + String(i));
      if (!off.length())
        off = server.arg("offTime" + String(i));

      if (on.length() && off.length()) {
        any = true;
        g_timerOn[i - 1]  = on;
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

  // SEARCH
  server.on("/search", HTTP_GET, []() {
    server.send(200, "text/html", searchModeHtml);
  });
  server.on("/search_submit", HTTP_GET, []() {
    String gap     = server.arg("gap");
    String dry     = server.arg("dryrun");
    String onTime  = server.arg("onTime");
    String offTime = server.arg("offTime");

    String days;
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "days") {
        if (days.length()) days += " ";
        days += server.arg(i);
      }
    }

    g_searchGap      = gap;
    g_searchDry      = dry;
    g_searchOnTime   = onTime;
    g_searchOffTime  = offTime;
    g_searchDays     = days;

    ws_sendPacketToSTM32("@SEARCH:SET:" + gap + ":" + dry + ":" +
                         onTime + ":" + offTime + ":" + days);

    String resp = "Search settings saved:\n";
    resp += "Gap: " + gap + "\n";
    resp += "DryRun: " + dry + "\n";
    resp += "ON Time: " + onTime + "\n";
    resp += "OFF Time: " + offTime + "\n";
    resp += "Days: " + days + "\n";
    server.send(200, "text/plain", resp);
  });

  // TWIST
  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });
  server.on("/twist_submit", HTTP_GET, []() {
    g_twistOnDur   = server.arg("onDuration");
    g_twistOffDur  = server.arg("offDuration");
    g_twistOnTime  = server.arg("onTime");
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

  // SEMI AUTO
  server.on("/semi", HTTP_GET, []() {
    server.send(200, "text/html", semiAutoModeHtml);
  });
  server.on("/semi_toggle", HTTP_GET, []() {
    g_motorState = !g_motorState;
    ws_setMotor(g_motorState);
    ws_sendPacketToSTM32(g_motorState ? "@SEMI:ON" : "@SEMI:OFF");
    server.send(200, "text/plain", g_motorState ? "ON" : "OFF");
  });

  // MOTOR STATUS
  server.on("/motor_status", HTTP_GET, []() {
    server.send(200, "text/plain", g_motorState ? "ON" : "OFF");
  });

  // WATER LEVEL
  server.on("/status", HTTP_GET, []() {
    if (g_waterLevel <= 0) ws_updateSimulatedWaterLevel();
    server.send(200, "application/json",
        "{\"level\":" + String(g_waterLevel) + "}");
  });

  // DEBUG STATE JSON
  server.on("/state.json", HTTP_GET, []() {
    String json = "{";
    json += "\"motor\":"; json += (g_motorState ? "true" : "false"); json += ",";
    json += "\"level\":"; json += g_waterLevel; json += ",";
    json += "\"search_gap\":\"" + g_searchGap + "\",";
    json += "\"search_dry\":\"" + g_searchDry + "\",";
    json += "\"search_onTime\":\"" + g_searchOnTime + "\",";
    json += "\"search_offTime\":\"" + g_searchOffTime + "\",";
    json += "\"search_days\":\"" + g_searchDays + "\",";
    json += "\"twist_onDur\":\"" + g_twistOnDur + "\",";
    json += "\"twist_offDur\":\"" + g_twistOffDur + "\",";
    json += "\"twist_onTime\":\"" + g_twistOnTime + "\",";
    json += "\"twist_offTime\":\"" + g_twistOffTime + "\",";
    json += "\"twist_days\":\"" + g_twistDays + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("HTTP server started");
}


// ===== LOOP HOOK =====
static void ws_handleCountdownLogic() {
  if (!g_countdownActive) return;
  if (millis() > g_countdownEnd) {
    g_countdownActive = false;
    ws_setMotor(g_countdownFinalMotor);
    ws_sendPacketToSTM32(g_countdownFinalMotor ?
                         "@COUNTDOWN:DONE:ON" : "@COUNTDOWN:DONE:OFF");
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
