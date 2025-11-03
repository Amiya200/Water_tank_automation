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

static unsigned long s_lastStatusRequest = 0;
static const unsigned long STATUS_INTERVAL_MS = 5UL * 60UL * 1000UL; // 5 minutes

// ===== INIT =====
void esp_uart_init() {
  Serial.begin(ESP_UART_BAUD_RATE);
  Serial.setTimeout(10);
  memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
  s_rxBufferIndex = 0;
  s_lastStatusRequest = millis();
  DBG_PRINTLN("UART Init OK");
}

// ===== SEND =====
void esp_uart_send(const char *message) {
  if (!message || !*message) return;
  String pkt = message;
  pkt.trim();
  if (!pkt.startsWith("@")) pkt = "@" + pkt;
  if (!pkt.endsWith("#")) pkt += "#";
  Serial.write(pkt.c_str(), pkt.length());
  Serial.flush();
  #if ESP_UART_ENABLE_DEBUG
    Serial.print("→ SENT: "); Serial.println(pkt);
  #endif
}

// ===== RECEIVE =====
bool esp_uart_receive(char *buffer, size_t bufferSize) {
  bool packetReceived = false;
  while (Serial.available()) {
    char c = Serial.read();
    if (c < 32 && c != '@' && c != '#') continue;
    if (s_rxBufferIndex < ESP_UART_RX_BUFFER_SIZE - 1) {
      s_rxBuffer[s_rxBufferIndex++] = c;
      if (c == '#') {
        s_rxBuffer[s_rxBufferIndex] = '\0';
        packetReceived = true;
        break;
      }
    } else {
      s_rxBufferIndex = 0;
      memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
      DBG_PRINTLN("RX overflow cleared");
    }
  }

  if (packetReceived) {
    if (s_rxBuffer[0] != '@') return false;
    if (strlen(s_rxBuffer) >= bufferSize) return false;
    strncpy(buffer, s_rxBuffer, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
    memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
    s_rxBufferIndex = 0;
    return true;
  }
  return false;
}

// ===== AUTO STATUS REQUEST =====
void esp_uart_requestStatus() {
  esp_uart_send("@REQ:STATUS#");
  s_lastStatusRequest = millis();
  DBG_PRINTLN("📡 Requested @REQ:STATUS#");
}

void esp_uart_autoStatusRequest() {
  unsigned long now = millis();
  if (now - s_lastStatusRequest >= STATUS_INTERVAL_MS)
    esp_uart_requestStatus();
}

// ===== COMMAND PARSER =====
void esp_uart_processCommand(const char *cmd) {
  if (!cmd) return;

  if (strstr(cmd, "@STATUS:")) {
    // Example: @STATUS:MOTOR:ON:LEVEL:3:MODE:TIMER#
    String pkt = String(cmd);
    pkt.replace("@STATUS:", "");
    pkt.replace("#", "");

    int motorIdx = pkt.indexOf("MOTOR:");
    int levelIdx = pkt.indexOf(":LEVEL:");
    int modeIdx  = pkt.indexOf(":MODE:");

    String motor = pkt.substring(motorIdx + 6, levelIdx);
    String level = pkt.substring(levelIdx + 7, modeIdx);
    String mode  = pkt.substring(modeIdx + 6);

    Serial.printf("📶 STATUS → Motor:%s | Level:%s | Mode:%s\n",
                  motor.c_str(), level.c_str(), mode.c_str());
    return;
  }

  else if (strstr(cmd, "@ACK:")) {
    DBG_PRINTLN(cmd);
  }

  else if (strstr(cmd, "@ERR:")) {
    DBG_PRINTLN(cmd);
  }

  else {
    DBG_PRINT("Unknown packet: %s\n", cmd);
  }
}
