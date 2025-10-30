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

<<<<<<< HEAD
// ====== ESP-01 HAS ONLY FEW PINS ======
// GPIO2 is the safest for relay on ESP-01
#ifndef MOTOR_PIN
  #define MOTOR_PIN 2
#endif
=======
#include "esp_uart_comm.h" // Include for UART communication functions

WebServer server(80);
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c

int simulatedWaterLevel = 70;
unsigned long countdownEndTime = 0;
bool countdownActive = false;
<<<<<<< HEAD
bool motorExpectedState = false;
unsigned long lastUpdate = 0;

// timer / mode globals
String countdownDuration = "";
=======
bool motorExpectedState = false;  // true = ON, false = OFF
unsigned long lastUpdate = 0; // Defined here

// Global variables to hold user input from different modes (for internal ESP use, not directly sent via UART)
String countdownDuration = ""; // This is now handled by durationMin in the handler
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
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
String searchQuery = ""; // This is now handled by gap, dryRun, days in the handler
String twistValue = "";  // This is now handled by onDuration, offDuration, onTime, offTime, days in the handler
String semiAutoOption = ""; // This is now handled by motorState in the handler
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
<<<<<<< HEAD
=======
    Serial.println("[Manual] Motor turned ON");
    esp_uart_send("MOTOR_ON"); // Send command to STM32
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
    server.send(200, "text/plain", "Motor turned ON");
  });
  server.on("/manual/off", HTTP_GET, []() {
    setMotor(false);
<<<<<<< HEAD
=======
    Serial.println("[Manual] Motor turned OFF");
    esp_uart_send("MOTOR_OFF"); // Send command to STM32
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
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

      char uartMessage[32]; // Buffer for UART message

      if (mode == "on") {
<<<<<<< HEAD
        setMotor(true);
        motorExpectedState = false; // turn OFF later
        server.send(200, "text/plain", "Motor ON for " + String(durationMin) + " min, then OFF.");
=======
        digitalWrite(MOTOR_PIN, HIGH);
        motorExpectedState = LOW;
        Serial.printf("[Countdown ON] Motor ON for %d min\n", durationMin);
        snprintf(uartMessage, sizeof(uartMessage), "COUNTDOWN_ON:%d", durationMin); // Format message
        esp_uart_send(uartMessage); // Send command to STM32
        server.send(200, "text/plain", "Motor ON for " + String(durationMin) + " min, then OFF.");
      } else if (mode == "off") {
        digitalWrite(MOTOR_PIN, LOW);
        motorExpectedState = HIGH;
        Serial.printf("[Countdown OFF] Motor OFF for %d min\n", durationMin);
        snprintf(uartMessage, sizeof(uartMessage), "COUNTDOWN_OFF:%d", durationMin); // Format message
        esp_uart_send(uartMessage); // Send command to STM32
        server.send(200, "text/plain", "Motor OFF for " + String(durationMin) + " min, then ON.");
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
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
    char uartMessage[64]; // Buffer for UART message

    // Send a command to clear existing timers on STM32 before sending new ones
    esp_uart_send("TIMER_CLEAR");
    delay(10); // Small delay

    for (int i = 1; i <= 5; i++) {
      String onTime = server.arg("on" + String(i));
      String offTime = server.arg("off" + String(i));
      if (onTime.length() > 0 && offTime.length() > 0) {
        hasValidSlot = true;
        responseMessage += "Slot " + String(i) + ": ON at " + onTime + ", OFF at " + offTime + "\n";
<<<<<<< HEAD

        if (i == 1) { timerStartTime1 = onTime; timerStopTime1 = offTime; }
        else if (i == 2) { timerStartTime2 = onTime; timerStopTime2 = offTime; }
        else if (i == 3) { timerStartTime3 = onTime; timerStopTime3 = offTime; }
        else if (i == 4) { timerStartTime4 = onTime; timerStopTime4 = offTime; }
        else if (i == 5) { timerStartTime5 = onTime; timerStopTime5 = offTime; }
=======
        
        // Store the timer settings in global variables (already present in original code)
        if (i == 1) {
          timerStartTime1 = onTime;
          timerStopTime1 = offTime;
        } else if (i == 2) {
          timerStartTime2 = onTime;
          timerStopTime2 = offTime;
        } else if (i == 3) {
          timerStartTime3 = onTime;
          timerStopTime3 = offTime;
        } else if (i == 4) {
          timerStartTime4 = onTime;
          timerStopTime4 = offTime;
        } else if (i == 5) {
          timerStartTime5 = onTime;
          timerStopTime5 = offTime;
        }

        // Format and send each timer slot to STM32
        // Example format: "TIMER_SET:1:08:00:17:30" (Slot:ON_HH:MM:OFF_HH:MM)
        snprintf(uartMessage, sizeof(uartMessage), "TIMER_SET:%d:%s:%s", i, onTime.c_str(), offTime.c_str());
        esp_uart_send(uartMessage);
        delay(10); // Small delay to ensure messages are sent sequentially
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
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

<<<<<<< HEAD
=======
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

    // Send to STM32
    char uartMessage[128];
    snprintf(uartMessage, sizeof(uartMessage), "SEARCH_GAP:%s", gap.c_str());
    esp_uart_send(uartMessage);
    delay(10);
    snprintf(uartMessage, sizeof(uartMessage), "SEARCH_DRYRUN:%s", dryRun.c_str());
    esp_uart_send(uartMessage);
    delay(10);
    snprintf(uartMessage, sizeof(uartMessage), "SEARCH_DAYS:%s", days.c_str());
    esp_uart_send(uartMessage);

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

>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "days") {
        days += server.arg(i) + " ";
      }
    }

    // just echo
    server.send(200, "text/plain", "Search settings saved.");
  });

<<<<<<< HEAD
  // ===== TWIST =====
  server.on("/twist", HTTP_GET, []() {
    server.send(200, "text/html", twistModeHtml);
  });
  server.on("/twist_submit", HTTP_GET, []() {
    // just echo your data like before
=======
    // Send to STM32
    char uartMessage[128];
    snprintf(uartMessage, sizeof(uartMessage), "TWIST_ONDUR:%s", onDuration.c_str());
    esp_uart_send(uartMessage);
    delay(10);
    snprintf(uartMessage, sizeof(uartMessage), "TWIST_OFFDUR:%s", offDuration.c_str());
    esp_uart_send(uartMessage);
    delay(10);
    snprintf(uartMessage, sizeof(uartMessage), "TWIST_ONTIME:%s", onTime.c_str());
    esp_uart_send(uartMessage);
    delay(10);
    snprintf(uartMessage, sizeof(uartMessage), "TWIST_OFFTIME:%s", offTime.c_str());
    esp_uart_send(uartMessage);
    delay(10);
    snprintf(uartMessage, sizeof(uartMessage), "TWIST_DAYS:%s", days.c_str());
    esp_uart_send(uartMessage);

>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
    server.send(200, "text/plain", "Settings received");
  });

  // ===== ERROR BOX =====
  server.on("/error_box", HTTP_GET, []() {
    server.send(200, "text/html", errorBoxHtml);
  });
  server.on("/error_submit", HTTP_ANY, []() {
    if (server.hasArg("message")) {
      errorMessage = server.arg("message");
<<<<<<< HEAD
=======
      Serial.println("[Error Box] Message: " + errorMessage);
      // You might want to send this error message to STM32 for logging or display
      char uartMessage[128];
      snprintf(uartMessage, sizeof(uartMessage), "ERROR_MSG:%s", errorMessage.c_str());
      esp_uart_send(uartMessage);
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
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
<<<<<<< HEAD
    setMotor(!motorState);
    server.send(200, "text/plain", motorState ? "ON" : "OFF");
=======
    motorState = !motorState;

    if (motorState) {
      digitalWrite(MOTOR_PIN, HIGH);
      Serial.println("[Semi-Auto] Motor STARTED");
      esp_uart_send("SEMI_ON"); // Send command to STM32
      server.send(200, "text/plain", "ON");
    } else {
      digitalWrite(MOTOR_PIN, LOW);
      Serial.println("[Semi-Auto] Motor STOPPED");
      esp_uart_send("SEMI_OFF"); // Send command to STM32
      server.send(200, "text/plain", "OFF");
    }
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
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
<<<<<<< HEAD
=======
    Serial.println("[Countdown] Motor toggled after countdown finished.");
    // Optionally, send a message to STM32 that countdown finished and motor state changed
    esp_uart_send(motorExpectedState ? "COUNTDOWN_END_ON" : "COUNTDOWN_END_OFF");
>>>>>>> b24165c607cf9fcc22a9d4bbc0fb7f02b6d9112c
    countdownActive = false;
  }
}

void handleClient() {
  server.handleClient();
  handleCountdownLogic();
}
