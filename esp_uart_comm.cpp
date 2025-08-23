#include "esp_uart_comm.h"

// Create a HardwareSerial object for communication with STM32
HardwareSerial SerialSTM32(ESP_UART_NUM);

// Internal buffer for receiving data
static char s_rxBuffer[ESP_UART_RX_BUFFER_SIZE];
static int s_rxBufferIndex = 0;

/**
 * @brief Initializes the UART communication with the STM32.
 *        Sets up the baud rate, pins, and internal receive buffer.
 */
void esp_uart_init() {
    SerialSTM32.begin(ESP_UART_BAUD_RATE, SERIAL_8N1, ESP_UART_RX_PIN, ESP_UART_TX_PIN);
    SerialSTM32.setRxBufferSize(ESP_UART_RX_BUFFER_SIZE); // Set RX buffer size for SerialSTM32
    Serial.println("ESP32 UART Comm: Initialized.");
    Serial.printf("  UART Num: %d, Baud: %d, RX Pin: %d, TX Pin: %d\n",
                  ESP_UART_NUM, ESP_UART_BAUD_RATE, ESP_UART_RX_PIN, ESP_UART_TX_PIN);

    // Clear the internal buffer
    memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
    s_rxBufferIndex = 0;
}

/**
 * @brief Sends a null-terminated string message to the STM32 via UART.
 *        Automatically appends the defined delimiter.
 * @param message The string to send.
 */
void esp_uart_send(const char* message) {
    Serial.printf("ESP32 -> STM32: %s%c\n", message, ESP_UART_DELIMITER); // Debug print
    SerialSTM32.print(message);
    SerialSTM32.write(ESP_UART_DELIMITER); // Append the delimiter
}

/**
 * @brief Checks for and retrieves a complete packet received from the STM32.
 *        A packet is considered complete when the ESP_UART_DELIMITER is received.
 *        The internal receive buffer is cleared after a successful read.
 * @param buffer Pointer to a char array where the received packet will be copied.
 * @param bufferSize The maximum size of the provided buffer (including null terminator).
 * @return true if a complete packet was received and copied, false otherwise.
 */
bool esp_uart_receive(char* buffer, size_t bufferSize) {
    bool packetReceived = false;

    while (SerialSTM32.available()) {
        char incomingByte = SerialSTM32.read();

        // Check for buffer overflow before storing the byte
        if (s_rxBufferIndex < (ESP_UART_RX_BUFFER_SIZE - 1)) {
            s_rxBuffer[s_rxBufferIndex++] = incomingByte;

            // Check if the received byte is the delimiter
            if (incomingByte == ESP_UART_DELIMITER) {
                s_rxBuffer[s_rxBufferIndex] = '\0'; // Null-terminate the received packet
                packetReceived = true;
                break; // Exit loop, packet is complete
            }
        } else {
            // Buffer overflow: discard current partial packet and reset
            Serial.println("ESP32 UART Comm: RX Buffer Overflow! Discarding partial packet.");
            s_rxBufferIndex = 0;
            memset(s_rxBuffer, 0, sizeof(s_rxBuffer)); // Clear buffer
            // Continue reading to clear the rest of the overflowing data if any
        }
    }

    if (packetReceived) {
        // Copy the received packet to the user-provided buffer
        size_t len = strlen(s_rxBuffer);
        if (len < bufferSize) {
            strncpy(buffer, s_rxBuffer, bufferSize - 1);
            buffer[bufferSize - 1] = '\0'; // Ensure null-termination
            Serial.printf("STM32 -> ESP32: %s\n", buffer); // Debug print
        } else {
            Serial.println("ESP32 UART Comm: User buffer too small for received packet. Discarding.");
            packetReceived = false; // Indicate failure to copy
        }

        // Reset internal buffer for the next packet, regardless of copy success
        memset(s_rxBuffer, 0, sizeof(s_rxBuffer));
        s_rxBufferIndex = 0;
    }

    return packetReceived;
}

/**
 * @brief Processes commands received from the STM32.
 *        This function is a placeholder; you'll implement your specific logic here.
 * @param command The received command string (null-terminated).
 */
void esp_uart_processCommand(const char* command) {
    // Example: React to water level updates from STM32
    if (strstr(command, "@10W#") != NULL) {
        Serial.println("ESP32: STM32 reports Water Level: 10%");
        // Add your logic here, e.g., update a display, send to cloud, etc.
    } else if (strstr(command, "@30W#") != NULL) {
        Serial.println("ESP32: STM32 reports Water Level: 30%");
    } else if (strstr(command, "@70W#") != NULL) {
        Serial.println("ESP32: STM32 reports Water Level: 70%");
    } else if (strstr(command, "@1:W#") != NULL) {
        Serial.println("ESP32: STM32 reports Water Level: 100%");
    } else if (strstr(command, "@DRY#") != NULL) {
        Serial.println("ESP32: STM32 reports DRY RUN detected!");
        // Potentially stop motor or send alert
    } else if (strstr(command, "@MOTOR_ON#") != NULL) {
        Serial.println("ESP32: STM32 reports Motor is ON.");
    } else if (strstr(command, "@MOTOR_OFF#") != NULL) {
        Serial.println("ESP32: STM32 reports Motor is OFF.");
    }
    // Add more processing for other commands STM32 might send
}
