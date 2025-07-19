
#include <WebServer.h>
#include <Arduino.h>
#include "web_server.h"
#include "html_content.h"

#include "manual_mode.h"
#include "countdown_mode.h"
#include "timer_mode.h"
#include "search_mode.h"
#include "twist_mode.h"
#include "error_box.h"
#include "semi_auto_mode.h"

WebServer server(80);

#define MOTOR_PIN 5
int simulatedWaterLevel = 70;
unsigned long countdownEndTime = 0;
bool countdownActive = false;
bool motorExpectedState = false;  // true = ON, false = OFF

// Global variables to hold user input from different modes
String countdownDuration = "";
String timerStartTime = "";
String timerStopTime = "";
String searchQuery = "";
String twistValue = "";
String semiAutoOption = "";
String errorMessage = "";

bool motorState = false;  // false = OFF, true = ON
void updateSimulatedWaterLevel() {
  simulatedWaterLevel = random(30, 100);  // or analogRead mapping
}

void start_webserver() {
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlContent);
  });

  // Manual mode
  server.on("/manual", HTTP_GET, []() {
    server.send(200, "text/html", manualModeHtml);
  });
  server.on("/manual/on", HTTP_GET, []() {
    digitalWrite(MOTOR_PIN, HIGH);
    Serial.println("[Manual] Pump turned ON");
    server.send(200, "text/plain", "Pump turned ON");
  });
  server.on("/manual/off", HTTP_GET, []() {
    digitalWrite(MOTOR_PIN, LOW);
    Serial.println("[Manual] Pump turned OFF");
    server.send(200, "text/plain", "Pump turned OFF");
  });

  // Countdown mode
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

      if (mode == "on") {
        digitalWrite(MOTOR_PIN, HIGH);
        motorExpectedState = LOW;
        Serial.printf("[Countdown ON] Motor ON for %d min\n", durationMin);
        server.send(200, "text/plain", "Motor ON for " + String(durationMin) + " min, then OFF.");
      } else if (mode == "off") {
        digitalWrite(MOTOR_PIN, LOW);
        motorExpectedState = HIGH;
        Serial.printf("[Countdown OFF] Motor OFF for %d min\n", durationMin);
        server.send(200, "text/plain", "Motor OFF for " + String(durationMin) + " min, then ON.");
      } else {
        server.send(400, "text/plain", "Invalid mode.");
      }
    } else {
      server.send(400, "text/plain", "Missing duration or mode.");
    }
  });


  // Timer mode

  server.on("/timer", HTTP_GET, []() {
    server.send(200, "text/html", timerModeHtml);
  });

  server.on("/timer/set", HTTP_GET, []() {
    if (server.hasArg("on") && server.hasArg("off")) {
      timerStartTime = server.arg("on");
      timerStopTime = server.arg("off");
      Serial.println("[Timer] ON at: " + timerStartTime + ", OFF at: " + timerStopTime);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  });




  // Search mode
  server.on("/search", HTTP_GET, []() {
    server.send(200, "text/html", searchModeHtml);
  });
  server.on("/search_submit", HTTP_GET, []() {
    String gap = server.arg("gap");
    String dryRun = server.arg("dryrun");
    String days = "";

    int totalArgs = server.args();
    for (int i = 0; i < totalArgs; i++) {
      if (server.argName(i) == "days") {
        days += server.arg(i) + " ";
      }
    }

    Serial.println("[Search Mode]");
    Serial.println("Testing Gap: " + gap);
    Serial.println("Dry Run Time: " + dryRun);
    Serial.println("Selected Days: " + days);

    server.send(200, "text/plain", "Search settings saved.");
  });


  // Twist mode
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
    Serial.println("ON Time: " + onTime + " - OFF Time: " + offTime);
    Serial.println("Days: " + days);

    server.send(200, "text/plain", "Settings received");
  });

  // Error box
  server.on("/error_box", HTTP_GET, []() {
    server.send(200, "text/html", errorBoxHtml);
  });
  server.on("/error_submit", HTTP_ANY, []() {
    if (server.hasArg("message")) {
      errorMessage = server.arg("message");
      Serial.println("[Error Box] Message: " + errorMessage);
      server.send(200, "text/plain", "Error message received");
    } else {
      server.send(400, "text/plain", "Missing error message");
    }
  });

  // Semi-auto mode
  server.on("/semi", HTTP_GET, []() {
    server.send(200, "text/html", semiAutoModeHtml);
  });

  server.on("/semi_toggle", HTTP_GET, []() {
    motorState = !motorState;

    if (motorState) {
      digitalWrite(MOTOR_PIN, HIGH);
      Serial.println("[Semi-Auto] Motor STARTED");
      server.send(200, "text/plain", "ON");
    } else {
      digitalWrite(MOTOR_PIN, LOW);
      Serial.println("[Semi-Auto] Motor STOPPED");
      server.send(200, "text/plain", "OFF");
    }
  });

  server.on("/motor_status", HTTP_GET, []() {
    server.send(200, "text/plain", motorState ? "ON" : "OFF");
  });




  // Water level status
  server.on("/status", HTTP_GET, []() {
    updateSimulatedWaterLevel();
    server.send(200, "application/json", "{\"level\": " + String(simulatedWaterLevel) + "}");
  });

  server.begin();
  Serial.println("HTTP server started");
}
void setMotor(bool state) {
  motorState = true;
  digitalWrite(MOTOR_PIN, state ? HIGH : LOW);
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
  handleCountdownLogic(); // ✅ actually runs countdown toggling
}




