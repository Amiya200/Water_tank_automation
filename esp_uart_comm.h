#ifndef ESP_UART_COMM_H
#define ESP_UART_COMM_H

#include <Arduino.h> // Required for basic Arduino functions like String, Serial, etc.
#include <HardwareSerial.h> // Required for HardwareSerial class

// --- Configuration ---
// Define the serial port for communication with STM32
// UART0: Used for USB Serial (Serial) - DO NOT USE FOR EXTERNAL DEVICES
// UART1: Pins 9 (RX), 10 (TX) - often used for external devices
// UART2: Pins 16 (RX), 17 (TX) - commonly used for external devices
// We'll use UART2 for this example.
#define ESP_UART_NUM 2 // Use UART2

// Define the baud rate for communication (must match STM32)
#define ESP_UART_BAUD_RATE 115200

// Define the RX and TX pins for the chosen UART
// These are typical default pins for Serial2 on ESP32.
// Adjust if your board uses different pins for UART2.
#define ESP_UART_RX_PIN 16 // GPIO16 (RX pin for Serial2)
#define ESP_UART_TX_PIN 17 // GPIO17 (TX pin for Serial2)

// Define the end-of-packet delimiter (must match STM32)
#define ESP_UART_DELIMITER '#'

// Buffer to store received data from STM32
#define ESP_UART_RX_BUFFER_SIZE 64 // Max expected packet size + 1 for null terminator

// --- Function Prototypes ---

/**
 * @brief Initializes the UART communication with the STM32.
 *        Sets up the baud rate, pins, and internal receive buffer.
 */
void esp_uart_init();

/**
 * @brief Sends a null-terminated string message to the STM32 via UART.
 *        Automatically appends the defined delimiter.
 * @param message The string to send.
 */
void esp_uart_send(const char* message);

/**
 * @brief Checks for and retrieves a complete packet received from the STM32.
 *        A packet is considered complete when the ESP_UART_DELIMITER is received.
 *        The internal receive buffer is cleared after a successful read.
 * @param buffer Pointer to a char array where the received packet will be copied.
 * @param bufferSize The maximum size of the provided buffer (including null terminator).
 * @return true if a complete packet was received and copied, false otherwise.
 */
bool esp_uart_receive(char* buffer, size_t bufferSize);

/**
 * @brief Processes commands received from the STM32.
 *        This function is a placeholder; you'll implement your specific logic here.
 * @param command The received command string (null-terminated).
 */
void esp_uart_processCommand(const char* command);

#endif // ESP_UART_COMM_H
