/*
  WiFi_functions.cpp

  This file defines functions used by examples connecting to,
  or creating, a WiFi network.

  Copyright 2020-2023 Metriful Ltd.
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
void connectToWiFi(const char * SSID, const char * password)
{
  WiFi.disconnect();
  #if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
  #endif
  uint8_t wStatus = WL_DISCONNECTED;
  while (wStatus != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to ");
    Serial.println(SSID);
    uint8_t statusChecks = 0;
    WiFi.begin(SSID, password);
    while ((wStatus != WL_CONNECTED) && (statusChecks < 8))
    {
      delay(1000);
      Serial.print(".");
      wStatus = WiFi.status();
      statusChecks++;
    }
    if (wStatus != WL_CONNECTED)
    {
      Serial.println("Failed.");
      WiFi.disconnect();
      delay(5000);
    }
  }
  Serial.println("Connected.");
}

// Configure the host as a WiFi access point, creating a WiFi network
// with specified network SSID (name), password and host IP address. 
bool createWiFiAP(const char * SSID, const char * password,
                  IPAddress hostIP)
{
  Serial.print("Creating access point named: ");
  Serial.println(SSID);
  #if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    IPAddress subnet(255, 255, 255, 0);
    bool success = WiFi.softAPConfig(hostIP, hostIP, subnet);
    success = success && WiFi.softAP(SSID, password);
  #else
    WiFi.config(hostIP);
    bool success = (WiFi.beginAP(SSID, password) == WL_AP_LISTENING);
  #endif
  return success;
}

// Provide a readable interpretation of the WiFi status.
// statusCode is the value returned by WiFi.status()
const char * interpret_WiFi_status(uint8_t statusCode)
{
  switch (statusCode)
  {
    case WL_CONNECTED:
      return "Connected";
    case WL_NO_SHIELD:
      return "No shield/module";
    case WL_IDLE_STATUS:
      return "Idle (temporary)";
    case WL_NO_SSID_AVAIL:
      return "No SSID available";
    case WL_SCAN_COMPLETED:
      return "Scan completed";
    case WL_CONNECT_FAILED:
      return "Connection failed";
    case WL_CONNECTION_LOST:
      return "Connection lost";
    case WL_DISCONNECTED:
      return "Disconnected";
  #if !defined(ESP8266) && !defined(ESP32)
    case WL_AP_CONNECTED:
      return "AP connected";
    case WL_AP_LISTENING:
      return "AP listening";
  #endif
    default:
      return "Unknown";
  }
}

// Get the IP address of the host.
// We need this function because the different board types
// do not have a consistent WiFi API.
IPAddress getIPaddress(bool isAccessPoint)
{
  if (isAccessPoint)
  {
    #if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
      return WiFi.softAPIP();
    #else
      return WiFi.localIP();
    #endif
  }
  else
  {
    return WiFi.localIP();
  }
}

// Either: connect to a wifi network, or create a new wifi network
// and assign the specified host IP address.
bool wifiCreateOrConnect(bool createWifiNetwork, bool waitForSerial,
                         const char * SSID, const char * password,
                         IPAddress hostIP)
{
  if (createWifiNetwork)
  {
    // The host generates its own WiFi network ("Access Point") with
    // a chosen static IP address
    if (!createWiFiAP(SSID, password, hostIP))
    {
      return false;
    }
  }
  else
  {
    // The host connects to an existing Wifi network
    
    // Wait for the serial port to start because the user must be able
    // to see the printed IP address in the serial monitor
    while (waitForSerial && (!Serial))
    {
      yield();
    }
    
    // Attempt to connect to the Wifi network and obtain the IP
    // address. Because the address is not known before this point,
    // a serial monitor must be used to display it to the user.
    connectToWiFi(SSID, password);
  }
 
  // Print the IP address: use this address in a browser to view the
  // generated web page
  Serial.print("View your page at http://");
  Serial.println(getIPaddress(createWifiNetwork));
  return true;
}


WiFiClient getClient(WiFiServer * server)
{
  #ifdef ESP8266
    return server->accept();
  #else
    return server->available();
  #endif
}

#endif
