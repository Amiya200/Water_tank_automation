#include "web_server.h"
#include "wifi_manager.h"
#include "uart.h"
#include "globals.h"
unsigned long lastUpdate = 0;  // ✅ Only one real definition




void setup() {
  Serial.begin(115200);
  uartInit(Serial, 115200);  // UART initialized for STM32
  wifi_init();
  start_webserver();
  randomSeed(analogRead(0));
}

void loop() {
  handleClient();
  uartHandleIncoming();  // For STM32 to ESP32 commands
}
