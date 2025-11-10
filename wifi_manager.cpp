#include <ESP8266WiFi.h>
#include "wifi_manager.h"

// ===== Access Point Credentials =====
const char* AP_SSID = "Water_Tank";
const char* AP_PASS = "123456789";

// ===== Optional Static IP for AP =====
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

// ===== Function Prototypes =====
void wifi_init();
void wifi_loop();

// ===== Initialization =====
void wifi_init() {
  Serial.println("\n[WiFi] Setting up Access Point...");

  // Ensure clean start
  WiFi.disconnect(true);
  delay(500);
  WiFi.mode(WIFI_AP);
  delay(200);

  // Set static IP for AP mode (helps stability)
  WiFi.softAPConfig(apIP, apIP, netMsk);

  // Start AP
  bool result = WiFi.softAP(AP_SSID, AP_PASS, 1, false, 4); // channel=1, open=false, max clients=4
  if (result) {
    Serial.printf("[WiFi] AP started: %s\n", AP_SSID);
  } else {
    Serial.println("[WiFi] Failed to start AP!");
  }

  // Print AP IP
  Serial.print("[WiFi] AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Reduce beacon and background scan intervals for stability
  wifi_set_sleep_type(LIGHT_SLEEP_T);
}

// ===== Runtime Loop (call from main loop()) =====
void wifi_loop() {
  static unsigned long lastCheck = 0;
  unsigned long now = millis();

  if (now - lastCheck > 10000) { // every 10 seconds
    lastCheck = now;

    // Auto-heal: if Wi-Fi AP becomes unresponsive, restart
    if (WiFi.softAPgetStationNum() == 0 && WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] No clients connected. Reinitializing AP...");
      WiFi.softAPdisconnect(true);
      delay(500);
      WiFi.softAP(AP_SSID, AP_PASS);
      Serial.println("[WiFi] AP restarted.");
    }
  }

  yield(); // prevent watchdog resets
}
