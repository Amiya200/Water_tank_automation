// web_server.cpp  (ESP-01 / ESP8266 version)
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
#include "error_box.h"
#include "semi_auto_mode.h"

// ====== ESP-01 HAS ONLY FEW PINS ======
// GPIO2 is the safest for relay on ESP-01
#ifndef MOTOR_PIN
  #define MOTOR_PIN 2
#endif

int simulatedWaterLevel = 70;
unsigned long countdownEndTime = 0;
bool countdownActive = false;
bool motorExpectedState = false;
unsigned long lastUpdate = 0;

// timer / mode globals
String countdownDuration = "";
String timerStartTime1 = "";
String timerStopTime1 = "";
String timerStartTime2 = "";
String timerStopTime2 = "";
String timerStartTime3 = "";
String timerStopTime3 = "";
String timerStartTime4 = "";
String timerStopTime4 = "";
String timerStartTime5 = "";
String timerStopTime5 = "";
String searchQuery = "";
String twistValue = "";
String semiAutoOption = "";
String errorMessage = "";
bool motorState = false;   // OFF

static void setMotor(bool state) {
  motorState = state;
  digitalWrite(MOTOR_PIN, state ? HIGH : LOW);
}

void updateSimulatedWaterLevel() {
  simulatedWaterLevel = random(30, 100);
}

void start_webserver() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  // ===== ROOT =====
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  // ===== MANUAL =====
  server.on("/manual", HTTP_GET, []() {
    server.send(200, "text/html", manualModeHtml);
  });
  server.on("/manual/on", HTTP_GET, []() {
    setMotor(true);
    server.send(200, "text/plain", "Motor turned ON");
  });
  server.on("/manual/off", HTTP_GET, []() {
    setMotor(false);
    server.send(200, "text/plain", "Motor turned OFF");
  });

  // ===== COUNTDOWN =====
  server.on("/countdown", HTTP_GET, []() {
    server.send(200, "text/html", countdownModeHtml);
  });
  server.on("/start_countdown", HTTP_GET, []() {
    if (server.hasArg("duration") && server.hasArg("mode")) {
      int durationMin = server.arg("duration").toInt();
      String mode = server.arg("mode");

      if (durationMin <= 0 || durationMin > 180) {
        server.send(400, "text/plain", "Invalid duration");
        return;
      }

      unsigned long durationMs = durationMin * 60UL * 1000UL;
      countdownEndTime = millis() + durationMs;
      countdownActive = true;

      if (mode == "on") {
        setMotor(true);
        motorExpectedState = false; // turn OFF later
        server.send(200, "text/plain", "Motor ON for " + String(durationMin) + " min, then OFF.");
      } else {
        setMotor(false);
        motorExpectedState = true;  // turn ON later
        server.send(200, "text/plain", "Motor OFF for " + String(durationMin) + " min, then ON.");
      }
    } else {
      server.send(400, "text/plain", "Missing duration or mode");
    }
  });

  // ===== TIMER =====
  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });
  server.on("/timer/set", HTTP_GET, []() {
    String responseMessage = "Timer settings received:\n";
    bool hasValidSlot = false;

    for (int i = 1; i <= 5; i++) {
      String onTime = server.arg("on" + String(i));
      String offTime = server.arg("off" + String(i));
      if (onTime.length() > 0 && offTime.length() > 0) {
        hasValidSlot = true;
        responseMessage += "Slot " + String(i) + ": ON at " + onTime + ", OFF at " + offTime + "\n";

        if (i == 1) { timerStartTime1 = onTime; timerStopTime1 = offTime; }
        else if (i == 2) { timerStartTime2 = onTime; timerStopTime2 = offTime; }
        else if (i == 3) { timerStartTime3 = onTime; timerStopTime3 = offTime; }
        else if (i == 4) { timerStartTime4 = onTime; timerStopTime4 = offTime; }
        else if (i == 5) { timerStartTime5 = onTime; timerStopTime5 = offTime; }
      }
    }

    if (hasValidSlot) {
      server.send(200, "text/plain", "OK\n" + responseMessage);
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  });

  // ===== SEARCH =====
  server.on("/search", HTTP_GET, []() {
    server.send(200, "text/html", searchModeHtml);
  });
  server.on("/search_submit", HTTP_GET, []() {
    String gap = server.arg("gap");
    String dryRun = server.arg("dryrun");
    String days = "";

    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "days") {
        days += server.arg(i) + " ";
      }
    }

    // just echo
    server.send(200, "text/plain", "Search settings saved.");
  });

  // ===== TWIST =====
  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });
  server.on("/twist_submit", HTTP_GET, []() {
    // just echo your data like before
    server.send(200, "text/plain", "Settings received");
  });

  // ===== ERROR BOX =====
  server.on("/error_box", HTTP_GET, []() {
    server.send(200, "text/html", errorBoxHtml);
  });
  server.on("/error_submit", HTTP_ANY, []() {
    if (server.hasArg("message")) {
      errorMessage = server.arg("message");
      server.send(200, "text/plain", "Error message received");
    } else {
      server.send(400, "text/plain", "Missing error message");
    }
  });

  // ===== SEMI AUTO =====
  server.on("/semi", HTTP_GET, []() {
    server.send(200, "text/html", semiAutoModeHtml);
  });
  server.on("/semi_toggle", HTTP_GET, []() {
    setMotor(!motorState);
    server.send(200, "text/plain", motorState ? "ON" : "OFF");
  });

  // ===== MOTOR STATUS =====
  server.on("/motor_status", HTTP_GET, []() {
    server.send(200, "text/plain", motorState ? "ON" : "OFF");
  });

  // ===== WATER LEVEL JSON =====
  server.on("/status", HTTP_GET, []() {
    updateSimulatedWaterLevel();
    server.send(200, "application/json",
      "{\"level\": " + String(simulatedWaterLevel) + "}");
  });

  server.begin();
  Serial.println("HTTP server started (ESP-01)");
}

static void handleCountdownLogic() {
  if (countdownActive && millis() > countdownEndTime) {
    setMotor(motorExpectedState);
    countdownActive = false;
  }
}

void handleClient() {
  server.handleClient();
  handleCountdownLogic();
}
