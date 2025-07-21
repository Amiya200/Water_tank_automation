// Updated web_server.cpp with UART structured packet integration and readable debug messages
#include <WebServer.h>
#include <Arduino.h>
#include "web_server.h"
#include "html_content.h"
#include "globals.h"
#include "manual_mode.h"
#include "countdown_mode.h"
#include "timer_mode.h"
#include "search_mode.h"
#include "twist_mode.h"
#include "error_box.h"
#include "semi_auto_mode.h"
#include "logging_page.h"
#include "uart.h"

WebServer server(80);

#define MOTOR_PIN 5

int simulatedWaterLevel = 70;
unsigned long countdownEndTime = 0;
bool countdownActive = false;
bool motorExpectedState = false;
bool motorState = false;

String countdownDuration = "";
String timerStartTime = "";
String timerStopTime = "";
String searchQuery = "";
String twistValue = "";
String semiAutoOption = "";
String errorMessage = "";

void updateSimulatedWaterLevel() {
  simulatedWaterLevel = random(30, 100);
  uint8_t data[] = { (uint8_t)simulatedWaterLevel };
  Serial.printf("[UART TX] Water level: %d\n", simulatedWaterLevel);
  uartSendPacket(0x02, data, sizeof(data));
}

void setMotor(bool state) {
  motorState = state;
  digitalWrite(MOTOR_PIN, state ? HIGH : LOW);
  uint8_t data[] = { (uint8_t)(state ? 1 : 0) };
  Serial.printf("[UART TX] Motor state: %s\n", state ? "ON" : "OFF");
  uartSendPacket(0x01, data, sizeof(data));
}

void start_webserver() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  server.on("/manual", HTTP_GET, []() {
    server.send(200, "text/html", manualModeHtml);
  });

  server.on("/manual/on", HTTP_GET, []() {
    setMotor(true);
    Serial.println("[Manual] Motor turned ON");
    server.send(200, "text/plain", "Motor turned ON");
  });

  server.on("/manual/off", HTTP_GET, []() {
    setMotor(false);
    Serial.println("[Manual] Motor turned OFF");
    server.send(200, "text/plain", "Motor turned OFF");
  });

  server.on("/countdown", HTTP_GET, []() {
    server.send(200, "text/html", countdownModeHtml);
  });

  server.on("/start_countdown", HTTP_GET, []() {
    if (server.hasArg("duration") && server.hasArg("mode")) {
      int durationMin = server.arg("duration").toInt();
      String mode = server.arg("mode");

      if (durationMin <= 0 || durationMin > 180) {
        server.send(400, "text/plain", "Invalid duration.");
        return;
      }

      unsigned long durationMs = durationMin * 60UL * 1000UL;
      countdownEndTime = millis() + durationMs;
      countdownActive = true;

      uint8_t modeVal = (mode == "on") ? 1 : 0;
      uint8_t data[] = { (uint8_t)durationMin, modeVal };
      Serial.printf("[UART TX] Countdown duration: %d min, mode: %s\n", durationMin, mode.c_str());
      uartSendPacket(0x03, data, sizeof(data));

      setMotor(modeVal == 1);
      motorExpectedState = !modeVal;
      server.send(200, "text/plain", "Countdown command received.");
    } else {
      server.send(400, "text/plain", "Missing duration or mode.");
    }
  });

  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });

  server.on("/timer/set", HTTP_GET, []() {
    if (server.hasArg("on") && server.hasArg("off")) {
      timerStartTime = server.arg("on");
      timerStopTime = server.arg("off");
      Serial.printf("[Timer] ON at: %s, OFF at: %s\n", timerStartTime.c_str(), timerStopTime.c_str());
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  });

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

    Serial.println("[Search Mode]");
    Serial.println("Gap: " + gap);
    Serial.println("Dry Run: " + dryRun);
    Serial.println("Days: " + days);
    server.send(200, "text/plain", "Search settings received");
  });

  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });

  server.on("/twist_submit", HTTP_GET, []() {
    String onDuration = server.arg("onDuration");
    String offDuration = server.arg("offDuration");
    String onTime = server.arg("onTime");
    String offTime = server.arg("offTime");
    String days = "";

    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "days") {
        days += server.arg(i) + " ";
      }
    }

    Serial.println("[Twist Mode]");
    Serial.println("On Duration: " + onDuration);
    Serial.println("Off Duration: " + offDuration);
    Serial.println("ON Time: " + onTime);
    Serial.println("OFF Time: " + offTime);
    Serial.println("Days: " + days);
    server.send(200, "text/plain", "Twist settings received");
  });

  server.on("/error_submit", HTTP_ANY, []() {
    if (server.hasArg("message")) {
      errorMessage = server.arg("message");
      Serial.println("[Error Box] Message: " + errorMessage);
      const char* msgStr = errorMessage.c_str();
      uartSendPacket(0x05, (uint8_t*)msgStr, strlen(msgStr));
      server.send(200, "text/plain", "Error message received");
    } else {
      server.send(400, "text/plain", "Missing error message");
    }
  });

  server.on("/semi_toggle", HTTP_GET, []() {
    motorState = !motorState;
    setMotor(motorState);
    Serial.println(motorState ? "[Semi-Auto] Motor STARTED" : "[Semi-Auto] Motor STOPPED");
    server.send(200, "text/plain", motorState ? "ON" : "OFF");
  });

  server.on("/motor_status", HTTP_GET, []() {
    server.send(200, "text/plain", motorState ? "ON" : "OFF");
  });

  server.on("/status", HTTP_GET, []() {
    updateSimulatedWaterLevel();
    server.send(200, "application/json", "{\"level\": " + String(simulatedWaterLevel) + "}");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void handleCountdownLogic() {
  if (countdownActive && millis() > countdownEndTime) {
    setMotor(motorExpectedState);
    Serial.println("[Countdown] Motor toggled after countdown finished.");
    countdownActive = false;
  }
}

void handleClient() {
  server.handleClient();
  handleCountdownLogic();
}
