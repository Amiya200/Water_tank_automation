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

String g_liveMode = "OFFMODE";
String g_motorStatus = "OFF";
int g_liveLevel = 0;

struct SettingsCache {
  int dryRunGap = 0;
  int testingGap = 0;
  int maxRun = 0;
  int retryCount = 0;  // NEW
  int lowVolt = 0;
  int highVolt = 0;
  int overLoad = 0;
  int underLoad = 0;
  int powerRestore = 0;
} g_settings;
struct TimerSlot {
  bool enabled = false;
  String days = "";
  String onTime = "";
  String offTime = "";
  int gap = 0;
};

TimerSlot g_timer[5];

struct CountdownCache {
  bool active = false;
  int duration = 0;
} g_countdown;

struct TwistCache {
  bool active = false;
  int onDuration = 0;
  int offDuration = 0;
  String onTime = "";
  String offTime = "";
  String days = "";
} g_twist;

void sendPacket(String pkt) {
  if (!pkt.startsWith("@")) pkt = "@" + pkt;
  if (!pkt.endsWith("#")) pkt += "#";

  esp_uart_send(pkt.c_str());
}

void requestStatus() {
  sendPacket("@STATUS#");
}

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
    String val = token.substring(eq + 1);

    int v = val.toInt();

    /* compact protocol */

    if (key == "D") g_settings.dryRunGap = v;
    else if (key == "T") g_settings.testingGap = v;
    else if (key == "M") g_settings.maxRun = v;
    else if (key == "RC") g_settings.retryCount = v;  // NEW
    else if (key == "LV") g_settings.lowVolt = v;
    else if (key == "HV") g_settings.highVolt = v;
    else if (key == "OL") g_settings.overLoad = v;
    else if (key == "UL") g_settings.underLoad = v;
    else if (key == "PR") g_settings.powerRestore = v;

    /* legacy protocol */

    else if (key == "dryRunGap") g_settings.dryRunGap = v;
    else if (key == "testingGap") g_settings.testingGap = v;
    else if (key == "retryCount") g_settings.retryCount = v;  // NEW
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


  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"motor\":\"" + g_motorStatus + "\",";
    json += "\"level\":" + String(g_liveLevel) + ",";
    json += "\"mode\":\"" + g_liveMode + "\"}";

    server.send(200, "application/json", json);
  });


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


  /* ================= SETTINGS ================= */

  server.on("/settings", HTTP_GET, []() {
    server.send(200, "text/html", settingsModeHtml);
  });

  server.on("/settings/get", HTTP_GET, []() {
    String json = "{";
    json += "\"dryRunGap\":" + String(g_settings.dryRunGap) + ",";
    json += "\"testingGap\":" + String(g_settings.testingGap) + ",";
    json += "\"maxRun\":" + String(g_settings.maxRun) + ",";
    json += "\"retryCount\":" + String(g_settings.retryCount) + ",";  // NEW
    json += "\"lowVolt\":" + String(g_settings.lowVolt) + ",";
    json += "\"highVolt\":" + String(g_settings.highVolt) + ",";
    json += "\"overLoad\":" + String(g_settings.overLoad) + ",";
    json += "\"underLoad\":" + String(g_settings.underLoad) + ",";
    json += "\"powerRestore\":" + String(g_settings.powerRestore) + "}";

    server.send(200, "application/json", json);
  });


  server.on("/settings/set", HTTP_GET, []() {
    String data = server.arg("data");

    parseSettings(data);

    sendPacket("@SET:" + data + "#");

    server.send(200, "text/plain", "Settings Updated");
  });


  server.begin();

  Serial.println("FULL SYSTEM READY - ALL 6 MODES ACTIVE");
}


/* ======================================================
   MAIN LOOP HANDLER
====================================================== */

void handleClient() {
  server.handleClient();

  char rxBuf[256];

  if (esp_uart_receive(rxBuf, sizeof(rxBuf), 10)) {
    String pkt = String(rxBuf);

    pkt.trim();

    if (pkt.startsWith("@STATUS:")) {
      parseStatus(pkt);
    }

    else if (pkt.startsWith("@SET:") || pkt.startsWith("@SETTINGS:")) {
      int pos = pkt.indexOf(':');

      pkt.remove(0, pos + 1);

      if (pkt.endsWith("#"))
        pkt.remove(pkt.length() - 1);

      parseSettings(pkt);
    }

    else if (pkt.startsWith("@SOK#")) {
      Serial.println("STM32 SETTINGS SAVED");
    }

    else if (pkt.startsWith("@COUNTDOWN:DONE")) {
      g_countdown.active = false;
    }
  }
}