/* 
   WiFi_functions.cpp

   This file defines functions used by examples connecting to, 
   or creating, a WiFi network.
   
   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include "host_pin_definitions.h"
#ifdef HAS_WIFI
#include "Arduino.h"
#include "WiFi_functions.h"

// Repeatedly attempt to connect to the WiFi network using the input
// network name (SSID) and password. 
void connectToWiFi(const char * SSID, const char * password) {
  WiFi.disconnect();
  #if defined(ESP8266) || defined(ESP32)
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
  #endif
  uint8_t wStatus = WL_DISCONNECTED;
  while (wStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to ");
    Serial.println(SSID);
    uint8_t statusChecks = 0;
    WiFi.begin(SSID, password);
    while ((wStatus != WL_CONNECTED) && (statusChecks < 8)) {
      delay(1000);
      Serial.print(".");
      wStatus = WiFi.status();
      statusChecks++;
    }
    if (wStatus != WL_CONNECTED) {
      Serial.println("Failed.");
      WiFi.disconnect();
      delay(5000);
    }
  }
  Serial.println("Connected.");
}

// Configure the host as a WiFi access point, creating a WiFi network with
// specified network SSID (name), password and host IP address. 
bool createWiFiAP(const char * SSID, const char * password, IPAddress hostIP) {
  Serial.print("Creating access point named: ");
  Serial.println(SSID);
  #if defined(ESP8266) || defined(ESP32)
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    IPAddress subnet(255,255,255,0);
    bool success = WiFi.softAP(SSID, password);
    delay(2000);
    success = success && WiFi.softAPConfig(hostIP, hostIP, subnet);
  #else
    WiFi.config(hostIP);
    bool success = (WiFi.beginAP(SSID, password) == WL_AP_LISTENING);
  #endif
  return success;
}

// Provide a readable interpretation of the WiFi status.
// statusCode is the value returned by WiFi.status()
const char * interpret_WiFi_status(uint8_t statusCode) {
  switch (statusCode) {
    case WL_CONNECTED:
      return "Connected";
    case WL_NO_SHIELD:
      return "No shield";
    case WL_IDLE_STATUS:
      return "Idle";
    case WL_NO_SSID_AVAIL:
      return "No SSID available";
    case WL_SCAN_COMPLETED:
      return "Scan completed";
    case WL_CONNECT_FAILED:
      return "Connect failed";
    case WL_CONNECTION_LOST:
      return "Connection lost";
    case WL_DISCONNECTED:
      return "Disconnected";
    default:
      return "Unknown";
  }
}

#endif
