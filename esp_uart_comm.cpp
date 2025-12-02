#include "esp_uart_comm.h"
#include "auto_mode.h"        // ⭐ NEW — AUTO MODE SYNC
#include <Arduino.h>
#include <string.h>

// ===== EXTERNAL GLOBALS =====
extern String g_motorStatus;
extern int    g_waterLevel;
extern String g_mode;

// ===== RX BUFFER =====
static char s_rxBuffer[ESP_UART_RX_BUFFER_SIZE];
static int  s_rxBufferIndex = 0;

// ===== STATUS TIMER =====
static unsigned long s_lastStatusRequest = 0;

// ===== DEBUG MACROS =====
#if ESP_UART_ENABLE_DEBUG
  #define DBG_PRINT(...)     Serial.printf(__VA_ARGS__)
  #define DBG_PRINTLN(...)   Serial.println(__VA_ARGS__)
#else
  #define DBG_PRINT(...)
  #define DBG_PRINTLN(...)
#endif


// ======================================================
//  INIT
// ======================================================
void esp_uart_init() {
  Serial.begin(ESP_UART_BAUD_RATE);
  Serial.setTimeout(10);

  memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
  s_rxBufferIndex = 0;

  s_lastStatusRequest = millis();

  DBG_PRINTLN("UART Init OK");
}


// ======================================================
//  SEND PACKET (Always @...# with CRLF)
// ======================================================
void esp_uart_send(const char *message) {
  if (!message || !*message) return;

  String pkt = message;
  pkt.trim();

  if (!pkt.startsWith("@")) pkt = "@" + pkt;
  if (!pkt.endsWith("#"))   pkt += "#";

  pkt += "\r\n";              // ⭐ Add CRLF

  Serial.write(pkt.c_str(), pkt.length());
  Serial.flush();

#if ESP_UART_ENABLE_DEBUG
  Serial.print("→ SENT: ");
  Serial.println(pkt);
#endif
}


// ======================================================
//  RECEIVER (robust, handles multiple packets)
// ======================================================
bool esp_uart_receive(char *buffer, size_t bufferSize) {
  bool packetReceived = false;

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\r' || c == '\n') continue;
    if (c < 32 && c != '@' && c != '#') continue;

    if (c == '@') {
      s_rxBufferIndex = 0;
      memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
    }

    if (s_rxBufferIndex < ESP_UART_RX_BUFFER_SIZE - 1) {
      s_rxBuffer[s_rxBufferIndex++] = c;
    }

    if (c == '#') {
      s_rxBuffer[s_rxBufferIndex] = '\0';

      if (strlen(s_rxBuffer) < bufferSize) {
        strncpy(buffer, s_rxBuffer, bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        packetReceived = true;
      }

      s_rxBufferIndex = 0;
      memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
      return true;
    }
  }

  return packetReceived;
}


// ======================================================
//  STATUS REQUEST
// ======================================================
void esp_uart_requestStatus() {
  esp_uart_send("STATUS");
  s_lastStatusRequest = millis();
  DBG_PRINTLN("Requested @STATUS#");
}


// ======================================================
//  AUTO STATUS REQUEST (10s motor ON, 10m motor OFF)
// ======================================================
void esp_uart_autoStatusRequest() {
  unsigned long now = millis();

  unsigned long interval =
      (g_motorStatus == "ON") ? 10000UL : (10UL * 60UL * 1000UL);

  if (now - s_lastStatusRequest >= interval) {
    esp_uart_requestStatus();
    s_lastStatusRequest = now;

    DBG_PRINT("Auto status request → Motor:");
    DBG_PRINTLN(g_motorStatus);
  }
}


// ======================================================
//  PARSE INCOMING PACKET (STATUS + AUTO MODE SYNC)
// ======================================================
void esp_uart_processCommand(const char *cmd) {
  if (!cmd) return;

  // ======================================================
  //  STATUS PACKET HANDLER
  // ======================================================
  if (strstr(cmd, "@STATUS:")) {

    String pkt = String(cmd);
    pkt.replace("@STATUS:", "");
    pkt.replace("#", "");

    int motorIdx = pkt.indexOf("MOTOR:");
    int levelIdx = pkt.indexOf(":LEVEL:");
    int modeIdx  = pkt.indexOf(":MODE:");

    if (motorIdx == -1 || levelIdx == -1 || modeIdx == -1)
      return;

    g_motorStatus = pkt.substring(motorIdx + 6, levelIdx);
    g_waterLevel  = pkt.substring(levelIdx + 7, modeIdx).toInt();
    g_mode        = pkt.substring(modeIdx + 6);

    // ⭐ AUTO MODE SYNC — Updates g_autoState inside auto_mode.h
    autoMode_applyStatusFromSTM32(g_mode);

    Serial.printf("📶 STATUS → Motor:%s | Level:%d | Mode:%s\n",
                  g_motorStatus.c_str(),
                  g_waterLevel,
                  g_mode.c_str());
    return;
  }

  // ======================================================
  //  ACK / ERR PACKETS
  // ======================================================
  else if (strstr(cmd, "@ACK:")) {
    DBG_PRINTLN(cmd);
  }

  else if (strstr(cmd, "@ERR:")) {
    DBG_PRINTLN(cmd);
  }

  // ======================================================
  //  UNKNOWN COMMAND
  // ======================================================
  else {
    DBG_PRINT("Unknown packet: %s\n", cmd);
  }
}
