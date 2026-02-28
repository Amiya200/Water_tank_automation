#include <ESP8266WiFi.h>
#include "wifi_manager.h"
const char* AP_SSID = "helonix";
const char* AP_PASS = "123456789";
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
void wifi_init();
void wifi_loop();
void wifi_init() {
  Serial.println("\n[WiFi] Setting up Access Point...");
  WiFi.disconnect(true);
  delay(500);
  WiFi.mode(WIFI_AP);
  delay(200);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  bool result = WiFi.softAP(AP_SSID, AP_PASS, 1, false, 4); // channel=1, open=false, max clients=4
  if (result) {
    Serial.printf("[WiFi] AP started: %s\n", AP_SSID);
  } else {
    Serial.println("[WiFi] Failed to start AP!");
  }
  Serial.print("[WiFi] AP IP address: ");
  Serial.println(WiFi.softAPIP());
  wifi_set_sleep_type(LIGHT_SLEEP_T);
}

void wifi_loop() {
  static unsigned long lastCheck = 0;
  unsigned long now = millis();
  if (now - lastCheck > 10000) { 
    lastCheck = now;
    if (WiFi.softAPgetStationNum() == 0 && WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] No clients connected. Reinitializing AP...");
      WiFi.softAPdisconnect(true);
      delay(500);
      WiFi.softAP(AP_SSID, AP_PASS);
      Serial.println("[WiFi] AP restarted.");
    }
  }
  yield(); 
}
