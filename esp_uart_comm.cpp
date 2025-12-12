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
//
//  New: supports optional timeout (ms). If timeoutMs == 0 behaves
//       like previous non-blocking call (returns immediately).
// ======================================================
bool esp_uart_receive(char *buffer, size_t bufferSize, unsigned long timeoutMs /* = 0 */) {
  unsigned long start = millis();
  bool packetReceived = false;

  // Helper lambda to consume available bytes and check for complete packet
  auto drainSerialOnce = [&]() -> bool {
    while (Serial.available()) {
      char c = Serial.read();

      // ignore common line endings and control noise
      if (c == '\r' || c == '\n') continue;
      if ((uint8_t)c < 32 && c != '@' && c != '#') continue;

      // start of frame resets buffer
      if (c == '@') {
        s_rxBufferIndex = 0;
        memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
      }

      if (s_rxBufferIndex < ESP_UART_RX_BUFFER_SIZE - 1) {
        s_rxBuffer[s_rxBufferIndex++] = c;
      }

      // end of frame
      if (c == '#') {
        s_rxBuffer[s_rxBufferIndex] = '\0';

        if ((int)strlen(s_rxBuffer) < (int)bufferSize) {
          strncpy(buffer, s_rxBuffer, bufferSize - 1);
          buffer[bufferSize - 1] = '\0';
          packetReceived = true;
        } else {
          // if local buffer too small, copy truncated but null-terminated
          strncpy(buffer, s_rxBuffer, bufferSize - 1);
          buffer[bufferSize - 1] = '\0';
          packetReceived = true;
        }

        // reset internal buffer for next frame
        s_rxBufferIndex = 0;
        memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
        return true;
      }
    }
    return false;
  };

  // If no timeout requested, just consume available bytes once and return immediately
  if (timeoutMs == 0) {
    drainSerialOnce();
    return packetReceived;
  }

  // Otherwise wait up to timeoutMs for a complete packet
  while (millis() - start < timeoutMs) {
    if (drainSerialOnce()) return true;
    delay(1);
  }

  // final attempt (in case partial data arrived right at the boundary)
  drainSerialOnce();
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
//  Simple SETTINGS parser used for logging / lightweight sync
//  Input: kvStr like "k1=v1;k2=v2;..."
// ======================================================
static void esp_uart_parseSettingsAndLog(const String &kvStr) {
  if (kvStr.length() == 0) return;
  int start = 0;
  while (start < kvStr.length()) {
    int semi = kvStr.indexOf(';', start);
    String token;
    if (semi == -1) { token = kvStr.substring(start); start = kvStr.length(); }
    else { token = kvStr.substring(start, semi); start = semi + 1; }
    token.trim();
    if (token.length() == 0) continue;
    int eq = token.indexOf('=');
    if (eq == -1) continue;
    String k = token.substring(0, eq);
    String v = token.substring(eq + 1);
    k.trim(); v.trim();
    // Basic URL decode for readability (handles %XX and +)
    String dec;
    for (size_t i=0; i<v.length(); i++) {
      char c = v[i];
      if (c == '+') dec += ' ';
      else if (c == '%' && i + 2 < v.length()) {
        char hi = v[i+1];
        char lo = v[i+2];
        char hex[3] = { hi, lo, 0 };
        char ch = (char) strtol(hex, NULL, 16);
        dec += ch;
        i += 2;
      } else dec += c;
    }

    Serial.printf("SETTING → %s = %s\n", k.c_str(), dec.c_str());
    // NOTE: if you later want to update local ESP modules (auto_mode etc.) here,
    // map known keys to setter functions (e.g., autoMode_setTwistOnDur(dec.toInt())).
  }
}


// ======================================================
//  PARSE INCOMING PACKET (STATUS + AUTO MODE SYNC + SETTINGS)
// ======================================================
void esp_uart_processCommand(const char *cmd) {
  if (!cmd) return;

  // ======================================================
  //  STATUS PACKET HANDLER
  //  Supports optional ":SETTINGS:" chunk at the end:
  //  @STATUS:...:MODE:XYZ:SETTINGS:k1=v1;k2=v2# 
  // ======================================================
  if (strstr(cmd, "@STATUS:")) {

    String pkt = String(cmd);
    pkt.replace("@STATUS:", "");
    pkt.replace("#", "");

    // check for settings piece
    int settingsIdx = pkt.indexOf(":SETTINGS:");
    String settingsPart = "";
    if (settingsIdx != -1) {
      settingsPart = pkt.substring(settingsIdx + 10); // after ":SETTINGS:"
      // cut settings portion off pkt so legacy parsing still works
      pkt = pkt.substring(0, settingsIdx);
    }

    int motorIdx = pkt.indexOf("MOTOR:");
    int levelIdx = pkt.indexOf(":LEVEL:");
    int modeIdx  = pkt.indexOf(":MODE:");

    if (motorIdx == -1 || levelIdx == -1 || modeIdx == -1)
      return;

    g_motorStatus = pkt.substring(motorIdx + 6, levelIdx);
    g_waterLevel  = pkt.substring(levelIdx + 7, modeIdx).toInt();
    g_mode        = pkt.substring(modeIdx + 6);

    // ⭐ AUTO MODE SYNC — Updates g_autoState inside auto_mode.h (if implemented)
    autoMode_applyStatusFromSTM32(g_mode);

    Serial.printf("📶 STATUS → Motor:%s | Level:%d | Mode:%s\n",
                  g_motorStatus.c_str(),
                  g_waterLevel,
                  g_mode.c_str());

    // parse settings if present (log/update local modules as needed)
    if (settingsPart.length()) {
      esp_uart_parseSettingsAndLog(settingsPart);
    }

    return;
  }

  // ======================================================
  //  ACK / ERR PACKETS
  // ======================================================
  else if (strstr(cmd, "@ACK:")) {
    Serial.print("ACK RX → ");
    Serial.println(cmd);
    // if ACK relates to SETTINGS, print friendly
    if (strstr(cmd, "@ACK:SETTINGS")) {
      Serial.println("STM32 acknowledged SETTINGS");
    }
    return;
  }

  else if (strstr(cmd, "@ERR:")) {
    Serial.print("ERR RX → ");
    Serial.println(cmd);
    return;
  }

  // ======================================================
  //  UNKNOWN / OTHER COMMANDS
  // ======================================================
  else {
    Serial.print("Unknown packet: ");
    Serial.println(cmd);
  }
}
