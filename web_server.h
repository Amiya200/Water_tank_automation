#pragma once
#include <WebServer.h>

extern WebServer server;
extern unsigned long lastUpdate;

// Log structure
struct SystemLog {
  String message;
  String type;
  unsigned long timestamp;
};

void start_webserver();
void handleClient();
void updateSimulatedWaterLevel();
void handleCountdownLogic();
void addToLog(const String& message, const String& type = "message");
