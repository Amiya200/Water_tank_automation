
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

// Global variables to hold user input from different modes
String countdownDuration = "";
String timerStartTime = "";
String timerStopTime = "";
String searchQuery = "";
String twistValue = "";
String semiAutoOption = "";
String errorMessage = "";

void updateSimulatedWaterLevel() {
    simulatedWaterLevel = random(30, 100); // or analogRead mapping
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
    server.on("/start_countdown", HTTP_ANY, []() {
        if (server.hasArg("duration")) {
            countdownDuration = server.arg("duration");
            Serial.println("[Countdown] Duration entered: " + countdownDuration);
            server.send(200, "text/plain", "Countdown started for " + countdownDuration + " minutes.");
        } else {
            server.send(400, "text/plain", "Missing duration parameter");
        }
    });

    // Timer mode
    server.on("/timer", HTTP_GET, []() {
        server.send(200, "text/html", timerModeHtml);
    });
    server.on("/set_timer", HTTP_ANY, []() {
        if (server.hasArg("start") && server.hasArg("stop")) {
            timerStartTime = server.arg("start");
            timerStopTime = server.arg("stop");
            Serial.println("[Timer] Start: " + timerStartTime + ", Stop: " + timerStopTime);
            server.send(200, "text/plain", "Timer set from " + timerStartTime + " to " + timerStopTime);
        } else {
            server.send(400, "text/plain", "Missing start or stop time");
        }
    });

    // Search mode
    server.on("/search", HTTP_GET, []() {
        server.send(200, "text/html", searchModeHtml);
    });
    server.on("/search_submit", HTTP_ANY, []() {
        if (server.hasArg("query")) {
            searchQuery = server.arg("query");
            Serial.println("[Search] Query: " + searchQuery);
            server.send(200, "text/plain", "Search received: " + searchQuery);
        } else {
            server.send(400, "text/plain", "Missing search query");
        }
    });

    // Twist mode
    server.on("/twist", HTTP_GET, []() {
        server.send(200, "text/html", twistModeHtml);
    });
    server.on("/twist_submit", HTTP_ANY, []() {
        if (server.hasArg("value")) {
            twistValue = server.arg("value");
            Serial.println("[Twist] Value: " + twistValue);
            server.send(200, "text/plain", "Twist value received: " + twistValue);
        } else {
            server.send(400, "text/plain", "Missing twist value");
        }
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
    server.on("/semi_submit", HTTP_ANY, []() {
        if (server.hasArg("option")) {
            semiAutoOption = server.arg("option");
            Serial.println("[Semi-Auto] Option: " + semiAutoOption);
            server.send(200, "text/plain", "Semi-auto option received: " + semiAutoOption);
        } else {
            server.send(400, "text/plain", "Missing semi-auto option");
        }
    });

    // Water level status
    server.on("/status", HTTP_GET, []() {
        updateSimulatedWaterLevel();
        server.send(200, "application/json", "{\"level\": " + String(simulatedWaterLevel) + "}");
    });

    server.begin();
    Serial.println("HTTP server started");
}

void handleClient() {
    server.handleClient();
}
