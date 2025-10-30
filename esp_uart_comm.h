#pragma once
#include <Arduino.h>

#define ESP_UART_RX_BUFFER_SIZE 128
#define ESP_UART_BAUD_RATE      115200
#define ESP_UART_DELIMITER      '#'
#define ESP_UART_ENABLE_DEBUG   0

void esp_uart_init();
void esp_uart_send(const char *message);
bool esp_uart_receive(char *buffer, size_t bufferSize);
void esp_uart_processCommand(const char *command);  // ok if empty
