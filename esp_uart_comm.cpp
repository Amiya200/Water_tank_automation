// esp_uart_comm.cpp  (for ESP-01 / ESP8266)
#include "esp_uart_comm.h"
#include <string.h>

static char s_rxBuffer[ESP_UART_RX_BUFFER_SIZE];
static int  s_rxBufferIndex = 0;

#if ESP_UART_ENABLE_DEBUG
  #define DBG_PRINT(...)  Serial.printf(__VA_ARGS__)
  #define DBG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DBG_PRINT(...)
  #define DBG_PRINTLN(...)
#endif

void esp_uart_init() {
  // on ESP-01, Serial is also the programming port
  Serial.begin(ESP_UART_BAUD_RATE);
  Serial.setTimeout(10);

  // clear buffer
  memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
  s_rxBufferIndex = 0;

  DBG_PRINTLN("ESP8266 UART Comm: Initialized");
}

void esp_uart_send(const char *message) {
  // Send to STM32 (or whatever MCU) on the only UART
  Serial.print(message);
  Serial.write(ESP_UART_DELIMITER);
  Serial.print("\r\n");  // ✅ add newline after every complete command
  DBG_PRINT("ESP8266 -> STM32: %s%c\n", message, ESP_UART_DELIMITER);
}

bool esp_uart_receive(char *buffer, size_t bufferSize) {
  bool packetReceived = false;

  while (Serial.available()) {
    char incomingByte = Serial.read();

    // prevent overflow
    if (s_rxBufferIndex < (ESP_UART_RX_BUFFER_SIZE - 1)) {
      s_rxBuffer[s_rxBufferIndex++] = incomingByte;

      if (incomingByte == ESP_UART_DELIMITER) {
        s_rxBuffer[s_rxBufferIndex] = '\0';
        packetReceived = true;
        break;
      }
    } else {
      DBG_PRINTLN("ESP8266 UART: RX overflow, discarding");
      s_rxBufferIndex = 0;
      memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
    }
  }

  if (packetReceived) {
    size_t len = strlen(s_rxBuffer);
    if (len < bufferSize) {
      strncpy(buffer, s_rxBuffer, bufferSize - 1);
      buffer[bufferSize - 1] = '\0';
      DBG_PRINT("STM32 -> ESP8266: %s\n", buffer);
    } else {
      DBG_PRINTLN("ESP8266 UART: user buffer too small");
      packetReceived = false;
    }

    memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
    s_rxBufferIndex = 0;
  }

  return packetReceived;
}

void esp_uart_processCommand(const char *command) {
  // Handle all known UART packets from STM32

  if (strstr(command, "@10W#")) {
    DBG_PRINTLN("Water Level: 10%");
  } else if (strstr(command, "@30W#")) {
    DBG_PRINTLN("Water Level: 30%");
  } else if (strstr(command, "@70W#")) {
    DBG_PRINTLN("Water Level: 70%");
  } else if (strstr(command, "@1:W#")) {
    DBG_PRINTLN("Water Level: 100%");
  } 
  
  else if (strstr(command, "@DRY#")) {
    DBG_PRINTLN("⚠️ DRY RUN detected!");
  } 

  // ==== MANUAL MODE ====
  else if (strstr(command, "@MANUAL:ON#")) {
    DBG_PRINTLN("Manual Mode → Motor ON");
  } else if (strstr(command, "@MANUAL:OFF#")) {
    DBG_PRINTLN("Manual Mode → Motor OFF");
  }

  // ==== COUNTDOWN MODE ====
  else if (strstr(command, "@COUNTDOWN:ON#")) {
    DBG_PRINTLN("Countdown started → Motor ON immediately");
  } else if (strstr(command, "@COUNTDOWN:DONE:OFF#")) {
    DBG_PRINTLN("Countdown finished → Motor turned OFF");
  }

  // ==== OTHER MODES ====
  else if (strstr(command, "@TIMER#")) {
    DBG_PRINTLN("Timer mode update received");
  } else if (strstr(command, "@SEARCH#")) {
    DBG_PRINTLN("Search mode update received");
  } else if (strstr(command, "@TWIST#")) {
    DBG_PRINTLN("Twist mode update received");
  }

  // Unrecognized packets
  else {
    DBG_PRINT("Unknown UART Packet: %s\n", command);
  }
}
