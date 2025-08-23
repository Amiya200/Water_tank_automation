#include "web_server.h"
#include "wifi_manager.h"
#include "esp_uart_comm.h"



void setup() {
    Serial.begin(115200);
    wifi_init();
    start_webserver();
    randomSeed(analogRead(0));  // Random seed for simulated values
}

void loop() {
    handleClient();

    // handleCountdownLogic();
    if (millis() - lastUpdate > 5000) {
        updateSimulatedWaterLevel();  // Update simulated water level
        lastUpdate = millis();
    }
}
