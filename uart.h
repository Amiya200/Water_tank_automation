// uart.h
#ifndef UART_H
#define UART_H

#include <Arduino.h>

extern HardwareSerial *uart;

void uartInit(HardwareSerial &serialPort, long baud);
void uartSendPacket(uint8_t cmd, const uint8_t *data, uint8_t len);
void uartHandleIncoming();

#endif
