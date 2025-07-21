// uart.cpp
#include "uart.h"

#define START_BYTE '@'
#define END_BYTE   '#'
#define MAX_BUFFER 8

HardwareSerial *uart = nullptr;
uint8_t rxBuffer[MAX_BUFFER];
uint8_t rxIndex = 0;
bool receiving = false;

void uartInit(HardwareSerial &serialPort, long baud) {
  uart = &serialPort;
  uart->begin(baud);
}

void uartSendPacket(uint8_t cmd, const uint8_t *data, uint8_t len) {
  if (!uart) return;

  uart->write(START_BYTE);
  uart->write(len);
  uart->write(cmd);

  uint8_t checksum = cmd;
  for (uint8_t i = 0; i < len; i++) {
    uart->write(data[i]);
    checksum ^= data[i];
  }

  uart->write(checksum);
  uart->write(END_BYTE);
}

void uartHandleIncoming() {
  if (!uart) return;

  while (uart->available()) {
    uint8_t byteIn = uart->read();

    if (!receiving) {
      if (byteIn == START_BYTE) {
        rxIndex = 0;
        receiving = true;
      }
    } else {
      if (byteIn == END_BYTE) {
        uint8_t len = rxBuffer[0];
        uint8_t cmd = rxBuffer[1];
        uint8_t checksum = cmd;

        for (int i = 0; i < len; i++) {
          checksum ^= rxBuffer[2 + i];
        }

        if (checksum == rxBuffer[2 + len]) {
          // ✅ Valid packet
          // Handle the command here if needed
        }

        receiving = false;
      } else {
        if (rxIndex < MAX_BUFFER - 1) {
          rxBuffer[rxIndex++] = byteIn;
        } else {
          receiving = false;
        }
      }
    }
  }
}
