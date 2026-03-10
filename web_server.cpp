#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WebServer.h>
static ESP8266WebServer server(80);
#else
#include <WebServer.h>
static WebServer server(80);
#endif

#include "web_server.h"
#include "timer_mode.h"
#include "settings_mode.h"
#include "countdown_mode.h"
#include "twist_mode.h"
#include "auto_mode.h"
#include "esp_uart_comm.h"

extern const char *htmlContent;

#ifndef MOTOR_PIN
#define MOTOR_PIN 2
#endif


/* ======================================================
   GLOBAL STATE
====================================================== */

String g_liveMode = "OFFMODE";
String g_motorStatus = "OFF";
int g_liveLevel = 0;


/* ================= SETTINGS CACHE ================= */

struct SettingsCache {
  int dryRunGap = 0;
  int testingGap = 0;
  int maxRun = 0;
  int retryCount = 0;
  int lowVolt = 0;
  int highVolt = 0;
  int overLoad = 0;
  int underLoad = 0;
  int powerRestore = 0;
} g_settings;


/* ================= TIMER CACHE ================= */

struct TimerSlot {
  bool enabled = false;
  String days = "";
  String onTime = "";
  String offTime = "";
  int gap = 0;
};

TimerSlot g_timer[5];


/* ================= COUNTDOWN CACHE ================= */

struct CountdownCache {
  bool active = false;
  int duration = 0;
} g_countdown;


/* ================= TWIST CACHE ================= */

struct TwistCache {
  bool active = false;
  int onDuration = 0;
  int offDuration = 0;
  String onTime = "";
  String offTime = "";
  String days = "";
} g_twist;


/* ======================================================
   UART COMMUNICATION
====================================================== */

void sendPacket(String pkt) {
  if (!pkt.startsWith("@"))
    pkt = "@" + pkt;

  if (!pkt.endsWith("#"))
    pkt += "#";

  esp_uart_send(pkt.c_str());
}

void requestStatus() {
  sendPacket("@STATUS#");
}


/* ======================================================
   MODE CONTROL
====================================================== */

void setMode(String newMode) {
  if (g_liveMode == newMode) {
    sendPacket("@" + newMode + ":OFF#");
    g_liveMode = "OFFMODE";
  } else {
    if (g_liveMode != "OFFMODE")
      sendPacket("@" + g_liveMode + ":OFF#");

    sendPacket("@" + newMode + ":ON#");

    g_liveMode = newMode;
  }
}


/* ======================================================
   STATUS PARSER
====================================================== */

void parseStatus(String pkt) {
  pkt.remove(0, 8);

  int mPos = pkt.indexOf("MOTOR:");
  int lPos = pkt.indexOf(":LEVEL:");
  int modePos = pkt.indexOf(":MODE:");

  if (mPos != -1 && lPos != -1 && modePos != -1) {
    g_motorStatus = pkt.substring(mPos + 6, lPos);

    int levelCode = pkt.substring(lPos + 7, modePos).toInt();

    switch (levelCode) {
      case 1: g_liveLevel = 25; break;
      case 2: g_liveLevel = 50; break;
      case 3: g_liveLevel = 75; break;
      case 4: g_liveLevel = 100; break;
      default: g_liveLevel = 0;
    }

    g_liveMode = pkt.substring(modePos + 6);
  }
}


/* ======================================================
   SETTINGS PARSER
====================================================== */

void parseSettings(String data) {
  int start = 0;

  while (start < data.length()) {
    int semi = data.indexOf(';', start);

    String token;

    if (semi == -1) {
      token = data.substring(start);
      start = data.length();
    } else {
      token = data.substring(start, semi);
      start = semi + 1;
    }

    int eq = token.indexOf('=');
    if (eq == -1) continue;

    String key = token.substring(0, eq);
    int v = token.substring(eq + 1).toInt();

    if (key == "dryRunGap") g_settings.dryRunGap = v;
    else if (key == "testingGap") g_settings.testingGap = v;
    else if (key == "maxRun") g_settings.maxRun = v;
    else if (key == "lowVolt") g_settings.lowVolt = v;
    else if (key == "highVolt") g_settings.highVolt = v;
    else if (key == "overLoad") g_settings.overLoad = v;
    else if (key == "underLoad") g_settings.underLoad = v;
    else if (key == "powerRestore") g_settings.powerRestore = v;
  }
}


/* ======================================================
   WEB SERVER
====================================================== */

void start_webserver() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });


  /* ===== STATUS ===== */

  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"motor\":\"" + g_motorStatus + "\",";
    json += "\"level\":" + String(g_liveLevel) + ",";
    json += "\"mode\":\"" + g_liveMode + "\"}";
    server.send(200, "application/json", json);
  });


  /* ===== MODE ===== */

  server.on("/auto_toggle", HTTP_GET, []() {
    setMode("AUTO");
    server.send(200, "text/plain", "OK");
  });
  server.on("/manual_toggle", HTTP_GET, []() {
    setMode("MANUAL");
    server.send(200, "text/plain", "OK");
  });
  server.on("/semi_toggle", HTTP_GET, []() {
    setMode("SEMIAUTO");
    server.send(200, "text/plain", "OK");
  });


  /* ===== TIMER ===== */

  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });


  server.on("/timer/set", HTTP_GET, []() {
    for (int i = 1; i <= 5; i++) {
      String slotKey = "slot" + String(i);

      if (!server.hasArg(slotKey))
        continue;

      bool enable = server.arg(slotKey).toInt();

      g_timer[i - 1].enabled = enable;

      g_timer[i - 1].days = server.arg("days" + String(i));
      g_timer[i - 1].onTime = server.arg("on" + String(i));
      g_timer[i - 1].offTime = server.arg("off" + String(i));

      int onH = g_timer[i - 1].onTime.substring(0, 2).toInt();
      int onM = g_timer[i - 1].onTime.substring(3, 5).toInt();

      int offH = g_timer[i - 1].offTime.substring(0, 2).toInt();
      int offM = g_timer[i - 1].offTime.substring(3, 5).toInt();

      sendPacket("@TIMER:SET:" + String(i) + ":" + g_timer[i - 1].days + ":" + String(onH) + ":" + String(onM) + ":" + String(offH) + ":" + String(offM) + ":" + String(enable ? 1 : 0) + "#");
    }

    bool timerActive = false;

    for (int i = 0; i < 5; i++) {
      if (g_timer[i].enabled) {
        timerActive = true;
        break;
      }
    }

    if (timerActive)
      setMode("TIMER");
    else
      setMode("OFFMODE");

    server.send(200, "text/plain", "Timer Updated");
  });

  /* ===== COUNTDOWN ===== */

  server.on("/countdown", HTTP_GET, []() {
    server.send(200, "text/html", countdownModeHtml);
  });

  server.on("/start_countdown", HTTP_GET, []() {
    int dur = server.arg("duration").toInt();

    g_countdown.duration = dur;
    g_countdown.active = true;

    sendPacket("@COUNTDOWN:ON:" + String(dur) + "#");

    setMode("COUNTDOWN");

    server.send(200, "text/plain", "Countdown Started");
  });


  server.on("/countdown_stop", HTTP_GET, []() {
    g_countdown.active = false;

    sendPacket("@COUNTDOWN:OFF#");

    setMode("OFFMODE");

    server.send(200, "text/plain", "Countdown Stopped");
  });


  /* ===== TWIST ===== */

  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });

  server.on("/twist_submit", HTTP_GET, []() {
    g_twist.onDuration = server.arg("onDuration").toInt();
    g_twist.offDuration = server.arg("offDuration").toInt();
    g_twist.onTime = server.arg("onTime");
    g_twist.offTime = server.arg("offTime");

    sendPacket("@TWIST:SET:" + String(g_twist.onDuration) + ":" + String(g_twist.offDuration) + ":" + g_twist.onTime + ":" + g_twist.offTime + "#");

    setMode("TWIST");

    server.send(200, "text/plain", "Twist Updated");
  });


  /* ===== SETTINGS ===== */

  server.on("/settings", HTTP_GET, []() {
    server.send(200, "text/html", settingsModeHtml);
  });

  server.on("/settings/set", HTTP_GET, []() {
    String data = server.arg("data");

    parseSettings(data);

    sendPacket("@SETTINGS:" + data + "#");

    server.send(200, "text/plain", "Settings Updated");
  });


  server.begin();

  Serial.println("FULL SYSTEM READY - ALL MODES ACTIVE");
}


/* ======================================================
   MAIN LOOP
====================================================== */

void handleClient() {
  server.handleClient();

  char rxBuf[256];

  if (esp_uart_receive(rxBuf, sizeof(rxBuf), 10)) {
    String pkt = String(rxBuf);

    pkt.trim();

    if (pkt.startsWith("@STATUS:"))
      parseStatus(pkt);

    else if (pkt.startsWith("@SETTINGS:")) {
      pkt.remove(0, 10);

      if (pkt.endsWith("#"))
        pkt.remove(pkt.length() - 1);

      parseSettings(pkt);
    }

    else if (pkt.startsWith("@COUNTDOWN:DONE"))
      g_countdown.active = false;
  }
}