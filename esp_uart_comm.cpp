// esp_uart_comm.cpp  (Improved)
// Handles stable UART bridge between ESP8266 <-> STM32 without junk bytes

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
  Serial.begin(ESP_UART_BAUD_RATE);
  Serial.setTimeout(10);
  memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
  s_rxBufferIndex = 0;
  DBG_PRINTLN("UART Init OK");
}

/**
 * Clean packet send — sends @CMD# exactly (no CRLF)
 */
void esp_uart_send(const char *message) {
  if (!message || !*message) return;

  // Ensure proper format
  String pkt = message;
  pkt.trim();

  // Add missing prefix/suffix if needed
  if (!pkt.startsWith("@")) pkt = "@" + pkt;
  if (!pkt.endsWith("#")) pkt += "#";

  // Send clean packet
  Serial.write(pkt.c_str(), pkt.length());
  Serial.flush();  // Ensure UART TX buffer fully sent

  #if ESP_UART_ENABLE_DEBUG
    Serial.print("\n→ SENT to STM32: ");
    Serial.println(pkt);
  #endif
}

/**
 * Receive a packet terminated by '#'
 * Returns true when a full packet is received
 */
bool esp_uart_receive(char *buffer, size_t bufferSize) {
  bool packetReceived = false;

  while (Serial.available()) {
    char incomingByte = Serial.read();

    // Skip non-printable garbage
    if (incomingByte < 32 && incomingByte != '@' && incomingByte != '#')
      continue;

    if (s_rxBufferIndex < (ESP_UART_RX_BUFFER_SIZE - 1)) {
      s_rxBuffer[s_rxBufferIndex++] = incomingByte;

      // Complete packet
      if (incomingByte == '#') {
        s_rxBuffer[s_rxBufferIndex] = '\0';
        packetReceived = true;
        break;
      }
    } else {
      // Overflow protection
      s_rxBufferIndex = 0;
      memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
      DBG_PRINTLN("UART RX overflow -> cleared");
    }
  }

  if (packetReceived) {
    // Validate format: must start with '@' and end with '#'
    if (s_rxBuffer[0] != '@') {
      DBG_PRINTLN("⚠️ Invalid packet prefix, discarded");
      memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
      s_rxBufferIndex = 0;
      return false;
    }

    size_t len = strlen(s_rxBuffer);
    if (len >= bufferSize) {
      DBG_PRINTLN("⚠️ Buffer too small, discarded");
      memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
      s_rxBufferIndex = 0;
      return false;
    }

    strncpy(buffer, s_rxBuffer, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';

    #if ESP_UART_ENABLE_DEBUG
      Serial.print("← RX from STM32: ");
      Serial.println(buffer);
    #endif

    memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
    s_rxBufferIndex = 0;
    return true;
  }

  return false;
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
