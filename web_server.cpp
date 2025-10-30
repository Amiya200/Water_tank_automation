// web_server.cpp  – ESP-01 / ESP8266
#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WebServer.h>
  static ESP8266WebServer server(80);
#else
  #include <WebServer.h>
  static WebServer server(80);
#endif

#include "web_server.h"
#include "html_content.h"
#include "manual_mode.h"
#include "countdown_mode.h"
#include "timer_mode.h"
#include "search_mode.h"
#include "twist_mode.h"
#include "semi_auto_mode.h"
// if you have error_box.h keep it, else comment its routes
// #include "error_box.h"

#include "esp_uart_comm.h"   // our UART helper

#ifndef MOTOR_PIN
  #define MOTOR_PIN 2
#endif

// ------------------ runtime/shared state ------------------
static bool   motorState           = false;
static int    simulatedWaterLevel  = 70;

// countdown
static bool   countdownActive      = false;
static unsigned long countdownEndTime = 0;
static bool   countdownWillSetMotorTo = false;

// timer (5 slots)
static String timerStartTime[5];
static String timerStopTime[5];

// search
static String search_gap;
static String search_dryrun;
static String search_days;

// twist
static String twist_onDur;
static String twist_offDur;
static String twist_onTime;
static String twist_offTime;
static String twist_days;

// error
static String lastErrorMsg;

// -----------------------------------------------------------
// helpers (define BEFORE we use them in lambdas)
// -----------------------------------------------------------
static void ws_setMotor(bool on) {
  motorState = on;
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, on ? HIGH : LOW);
}

// Arduino sometimes auto-prototypes weirdly, so we give it
// a name that won’t collide and we define it here.
static void ws_updateSimulatedWaterLevel() {
  // if you later get real level from STM32, you can remove this
  simulatedWaterLevel = random(30, 100);
}

// wrap & send packet to STM32 (UART)
static void ws_sendPacketToSTM32(const String &payload) {
  // payload should start with '@'
  // esp_uart_send() will append '#'
  esp_uart_send(payload.c_str());
}

// apply UART packet (received from STM32) to our web-visible state
static void ws_applyUartPacketToWeb(const char *rawIn) {
  if (!rawIn || !rawIn[0]) return;

  String pkt = String(rawIn);
  pkt.trim();

  // strip trailing '#'
  if (pkt.endsWith("#")) pkt.remove(pkt.length() - 1);
  // strip leading '@'
  if (pkt.startsWith("@")) pkt.remove(0, 1);

  // ---- backward compat motor ----
  if (pkt == "MOTOR_ON")  { ws_setMotor(true);  return; }
  if (pkt == "MOTOR_OFF") { ws_setMotor(false); return; }

  // ---- backward compat water ----
  if (pkt == "10W") { simulatedWaterLevel = 10;  return; }
  if (pkt == "30W") { simulatedWaterLevel = 30;  return; }
  if (pkt == "70W") { simulatedWaterLevel = 70;  return; }
  if (pkt == "1:W") { simulatedWaterLevel = 100; return; }

  // ---- new unified format ----
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
        timerStartTime[i] = "";
        timerStopTime[i]  = "";
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
          timerStartTime[slot - 1] = on;
          timerStopTime[slot - 1]  = off;
        }
      }
    }
    return;
  }

  // SEARCH:SET:<gap>:<dry>:<days>
  if (cmd == "SEARCH") {
    if (rest.startsWith("SET:")) {
      String body = rest.substring(4);
      int a = body.indexOf(':');
      int b = (a == -1) ? -1 : body.indexOf(':', a + 1);
      if (a != -1 && b != -1) {
        search_gap    = body.substring(0, a);
        search_dryrun = body.substring(a + 1, b);
        search_days   = body.substring(b + 1);
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
        twist_onDur   = body.substring(0, a);
        twist_offDur  = body.substring(a + 1, b);
        twist_onTime  = body.substring(b + 1, c);
        twist_offTime = body.substring(c + 1, d);
        twist_days    = body.substring(d + 1);
      }
    }
    return;
  }

  if (cmd == "ERROR") {
    lastErrorMsg = rest;
    return;
  }
}

// -----------------------------------------------------------
// HTTP ROUTES
// -----------------------------------------------------------
void start_webserver() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  // root
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  // manual
  server.on("/manual", HTTP_GET, []() {
    server.send(200, "text/html", manualModeHtml);
  });
  server.on("/manual/on", HTTP_GET, []() {
    ws_setMotor(true);
    ws_sendPacketToSTM32("@MANUAL:ON");
    server.send(200, "text/plain", "Motor turned ON");
  });
  server.on("/manual/off", HTTP_GET, []() {
    ws_setMotor(false);
    ws_sendPacketToSTM32("@MANUAL:OFF");
    server.send(200, "text/plain", "Motor turned OFF");
  });

  // countdown screen
  server.on("/countdown", HTTP_GET, []() {
    server.send(200, "text/html", countdownModeHtml);
  });

  // countdown submit
  server.on("/countdown_submit", HTTP_GET, []() {
    if (!server.hasArg("duration") || !server.hasArg("mode")) {
      server.send(400, "text/plain", "Missing duration or mode");
      return;
    }

    int durationMin = server.arg("duration").toInt();
    String mode = server.arg("mode");

    countdownEndTime = millis() + (unsigned long)durationMin * 60UL * 1000UL;
    countdownActive = true;

    if (mode == "on") {
      ws_setMotor(true);
      countdownWillSetMotorTo = false;
      ws_sendPacketToSTM32("@COUNTDOWN:ON:" + String(durationMin));
      server.send(200, "text/plain", "Motor ON for " + String(durationMin) + " min, then OFF.");
    } else {
      ws_setMotor(false);
      countdownWillSetMotorTo = true;
      ws_sendPacketToSTM32("@COUNTDOWN:OFF:" + String(durationMin));
      server.send(200, "text/plain", "Motor OFF for " + String(durationMin) + " min, then ON.");
    }
  });

  // timer
  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });
  server.on("/timer/set", HTTP_GET, []() {
    ws_sendPacketToSTM32("@TIMER:CLEAR");
    delay(5);

    bool anyValid = false;
    String resp = "Timer settings:\n";

    for (int i = 1; i <= 5; i++) {
      String on  = server.arg("on"  + String(i));
      String off = server.arg("off" + String(i));
      if (on.length() && off.length()) {
        anyValid = true;
        timerStartTime[i - 1] = on;
        timerStopTime[i - 1]  = off;

        String pkt = "@TIMER:SET:" + String(i) + ":" + on + ":" + off;
        ws_sendPacketToSTM32(pkt);
        delay(3);

        resp += "Slot " + String(i) + ": " + on + " -> " + off + "\n";
      }
    }

    if (anyValid) server.send(200, "text/plain", resp);
    else          server.send(400, "text/plain", "Missing parameters");
  });

  // search
  server.on("/search", HTTP_GET, []() {
    server.send(200, "text/html", searchModeHtml);
  });
  server.on("/search_submit", HTTP_GET, []() {
    String gap    = server.arg("gap");
    String dryrun = server.arg("dryrun");
    String days;

    int totalArgs = server.args();
    for (int i = 0; i < totalArgs; i++) {
      if (server.argName(i) == "days") {
        if (days.length()) days += " ";
        days += server.arg(i);
      }
    }

    search_gap    = gap;
    search_dryrun = dryrun;
    search_days   = days;

    String pkt = "@SEARCH:SET:" + gap + ":" + dryrun + ":" + days;
    ws_sendPacketToSTM32(pkt);

    server.send(200, "text/plain", "Search settings received.");
  });

  // twist
  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });
  server.on("/twist_submit", HTTP_GET, []() {
    twist_onDur   = server.arg("onDuration");
    twist_offDur  = server.arg("offDuration");
    twist_onTime  = server.arg("onTime");
    twist_offTime = server.arg("offTime");

    String days;
    int totalArgs = server.args();
    for (int i = 0; i < totalArgs; i++) {
      if (server.argName(i) == "days") {
        if (days.length()) days += " ";
        days += server.arg(i);
      }
    }
    twist_days = days;

    String pkt = "@TWIST:SET:" + twist_onDur + ":" + twist_offDur + ":" +
                 twist_onTime + ":" + twist_offTime + ":" + twist_days;
    ws_sendPacketToSTM32(pkt);

    server.send(200, "text/plain", "Twist settings received.");
  });

  // semi auto
  server.on("/semi", HTTP_GET, []() {
    server.send(200, "text/html", semiAutoModeHtml);
  });
  server.on("/semi_toggle", HTTP_GET, []() {
    motorState = !motorState;
    ws_setMotor(motorState);
    if (motorState) ws_sendPacketToSTM32("@SEMI:ON");
    else            ws_sendPacketToSTM32("@SEMI:OFF");
    server.send(200, "text/plain", motorState ? "ON" : "OFF");
  });

  // motor status
  server.on("/motor_status", HTTP_GET, []() {
    server.send(200, "text/plain", motorState ? "ON" : "OFF");
  });

  // water/status JSON
  server.on("/status", HTTP_GET, []() {
    if (simulatedWaterLevel <= 0) {
      ws_updateSimulatedWaterLevel();   // <-- FIXED NAME + defined above
    }
    server.send(200, "application/json",
      "{\"level\": " + String(simulatedWaterLevel) + "}");
  });

  // whole state
  server.on("/state.json", HTTP_GET, []() {
    String json = "{";
    json += "\"motor\":"; json += (motorState ? "true" : "false"); json += ",";
    json += "\"level\":"; json += simulatedWaterLevel; json += ",";
    json += "\"search_gap\":\"" + search_gap + "\",";
    json += "\"search_dry\":\"" + search_dryrun + "\",";
    json += "\"search_days\":\"" + search_days + "\",";
    json += "\"twist_onDur\":\"" + twist_onDur + "\",";
    json += "\"twist_offDur\":\"" + twist_offDur + "\",";
    json += "\"twist_onTime\":\"" + twist_onTime + "\",";
    json += "\"twist_offTime\":\"" + twist_offTime + "\",";
    json += "\"twist_days\":\"" + twist_days + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("HTTP server started");
}

// -----------------------------------------------------------
// periodic
// -----------------------------------------------------------
static void ws_handleCountdownLogic() {
  if (!countdownActive) return;
  if (millis() > countdownEndTime) {
    countdownActive = false;
    ws_setMotor(countdownWillSetMotorTo);
    if (countdownWillSetMotorTo)
      ws_sendPacketToSTM32("@COUNTDOWN:DONE:ON");
    else
      ws_sendPacketToSTM32("@COUNTDOWN:DONE:OFF");
  }
}

void handleClient() {
  server.handleClient();
  ws_handleCountdownLogic();

  // poll UART every loop
  char rxBuf[128];
  if (esp_uart_receive(rxBuf, sizeof(rxBuf))) {
    Serial.print("UART RX -> ");
    Serial.println(rxBuf);
    ws_applyUartPacketToWeb(rxBuf);
  }
}
