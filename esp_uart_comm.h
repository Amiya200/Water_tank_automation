// esp_uart_comm.h  (for ESP-01 / ESP8266)
#ifndef ESP_UART_COMM_H
#define ESP_UART_COMM_H

#include <Arduino.h>

/*
 * ESP-01 has only Serial (UART0), so we talk to STM32 on Serial.
 * Debug prints must be optional, otherwise they'll mix with packets.
 */

// ====== USER TUNABLES ======
#define ESP_UART_BAUD_RATE     115200
#define ESP_UART_DELIMITER     '#'
#define ESP_UART_RX_BUFFER_SIZE 64

// set to 1 only if you are NOT connecting STM32 yet and want to see logs
#define ESP_UART_ENABLE_DEBUG  0

void esp_uart_init();
void esp_uart_send(const char *message);
bool esp_uart_receive(char *buffer, size_t bufferSize);
void esp_uart_processCommand(const char *command);

#endif // ESP_UART_COMM_H
