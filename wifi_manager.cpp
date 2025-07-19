/*
#include <ESP8266WiFi.h>
#include "wifi_manager.h"
*/

// /*

#include <WiFi.h>
#include "wifi_manager.h"
// */


const char* ap_ssid = "ESP8266_Water_Tank"; // SSID for the Access Point
const char* ap_password = "123456789"; // Password for the Access Point
void wifi_init() {
    Serial.println("Setting up Access Point...");
    // Set up the Access Point
    WiFi.softAP(ap_ssid, ap_password);
    Serial.print("Access Point \"");
    Serial.print(ap_ssid);
    Serial.println("\" started");
    // Print the IP address of the Access Point
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
}


// const char* ssid = "YOUR_SSID"; // Replace with your Wi-Fi SSID
// const char* password = "YOUR_PASSWORD"; // Replace with your Wi-Fi password
//    void wifi_init() {
//        Serial.println("Connecting to Wi-Fi...");
       
//        // Set static IP address
//        IPAddress local_IP(192, 168, 1, 100); // Set your desired static IP
//        IPAddress gateway(192, 168, 1, 1); // Set your gateway (usually your router's IP)
//        IPAddress subnet(255, 255, 255, 0); // Set your subnet mask

//        // Start Wi-Fi with static IP
//        if (!WiFi.config(local_IP, gateway, subnet)) {
//            Serial.println("STA Failed to configure");
//        }
       
//        WiFi.begin(ssid, password);
       
//        while (WiFi.status() != WL_CONNECTED) {
//            delay(500);
//            Serial.print(".");
//        }
//        Serial.println(" connected!");
//        Serial.print("IP address: ");
//        Serial.println(WiFi.localIP());
//    }
   