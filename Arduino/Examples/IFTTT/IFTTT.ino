/* 
   IFTTT.ino
   
   Example code for sending data from the Metriful MS430 to IFTTT.com 
   
   This example is designed for the following WiFi enabled hosts:
   * Arduino Nano 33 IoT
   * Arduino MKR WiFi 1010
   * ESP8266 boards (e.g. Wemos D1, NodeMCU)
   * ESP32 boards (e.g. DOIT DevKit v1)
   
   Environmental data values are periodically measured and compared with
   a set of user-defined thresholds. If any values go outside the allowed
   ranges, an HTTP POST request is sent to IFTTT.com, triggering an alert
   email to your inbox, with customizable text. 
   This example requires a WiFi network and internet connection.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>
#include <WiFi_functions.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// The details of the WiFi network:
char SSID[] = "PUT WIFI NETWORK NAME HERE IN QUOTES"; // network SSID (name)
char password[] = "PUT WIFI PASSWORD HERE IN QUOTES"; // network password

// Define the details of variables for monitoring.
// The seven fields are:
// {Name, measurement unit, high threshold, low threshold, 
// initial inactive cycles (2), advice when high, advice when low}
ThresholdSetting_t humiditySetting = {"humidity","%",60,30,2,
                   "Reduce moisture sources.","Start the humidifier."};
ThresholdSetting_t airQualitySetting = {"air quality index","",250,-1,2,
                   "Improve ventilation and reduce sources of VOCs.",""};
// Change these values if Fahrenheit output temperature is selected in Metriful_sensor.h
ThresholdSetting_t temperatureSetting = {"temperature",CELSIUS_SYMBOL,24,18,2,
                   "Turn on the fan.","Turn on the heating."};

// An inactive period follows each alert, during which the same alert 
// will not be generated again - this prevents too many emails/alerts.
// Choose the period as a number of readout cycles (each 5 minutes) 
// e.g. for a 2 hour period, choose inactiveWaitCycles = 24
uint16_t inactiveWaitCycles = 24;

// IFTTT.com settings

// You must set up a free account on IFTTT.com and create a Webhooks 
// applet before using this example. This is explained further in the
// instructions in the GitHub Readme, or in the User Guide.

#define WEBHOOKS_KEY "PASTE YOUR KEY HERE WITHIN QUOTES"
#define IFTTT_EVENT_NAME "PASTE YOUR EVENT NAME HERE WITHIN QUOTES"

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

#if !defined(HAS_WIFI)
#error ("This example program has been created for specific WiFi enabled hosts only.")
#endif

// Measure the environment data every 300 seconds (5 minutes). This is 
// adequate for long-term monitoring.
uint8_t cycle_period = CYCLE_PERIOD_300_S;

WiFiClient client;

// Buffers for assembling the http POST requests
char postBuffer[400] = {0};
char fieldBuffer[120] = {0};

// Structs for data
AirData_t airData = {0};
AirQualityData_t airQualityData = {0};


void setup() {
  // Initialize the host's pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 

  connectToWiFi(SSID, password);

  // Enter cycle mode
  TransmitI2C(I2C_ADDRESS, CYCLE_TIME_PERIOD_REG, &cycle_period, 1);
  ready_assertion_event = false;
  TransmitI2C(I2C_ADDRESS, CYCLE_MODE_CMD, 0, 0);
}


void loop() {

  // Wait for the next new data release, indicated by a falling edge on READY
  while (!ready_assertion_event) {
    yield();
  }
  ready_assertion_event = false;

  // Read the air data and air quality data
  ReceiveI2C(I2C_ADDRESS, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);
  ReceiveI2C(I2C_ADDRESS, AIR_QUALITY_DATA_READ, (uint8_t *) &airQualityData, AIR_QUALITY_DATA_BYTES);

  // Check that WiFi is still connected
  uint8_t wifiStatus = WiFi.status();
  if (wifiStatus != WL_CONNECTED) {
    // There is a problem with the WiFi connection: attempt to reconnect.
    Serial.print("Wifi status: ");
    Serial.println(interpret_WiFi_status(wifiStatus));
    connectToWiFi(SSID, password);
    ready_assertion_event = false;
  }

  // Process temperature value and convert if using Fahrenheit
  float temperature = convertEncodedTemperatureToFloat(airData.T_C_int_with_sign, airData.T_C_fr_1dp);
  #ifdef USE_FAHRENHEIT
    temperature = convertCtoF(temperature);
  #endif

  // Send an alert to IFTTT if a variable is outside the allowed range
  // Just use the integer parts of values (ignore fractional parts)
  checkData(&temperatureSetting, (int32_t) temperature);
  checkData(&humiditySetting, (int32_t) airData.H_pc_int);
  checkData(&airQualitySetting, (int32_t) airQualityData.AQI_int);
}


// Compare the measured value to the chosen thresholds and create an
// alert if the value is outside the allowed range. After triggering
// an alert, it cannot be re-triggered within the chosen number of cycles.
void checkData(ThresholdSetting_t * setting, int32_t value) {

  // Count down to when the monitoring is active again:
  if (setting->inactiveCount > 0) {
    setting->inactiveCount--;
  }

  if ((value > setting->thresHigh) && (setting->inactiveCount == 0)) {
    // The variable is above the high threshold
    setting->inactiveCount = inactiveWaitCycles;
    sendAlert(setting, value, true);
  }
  else if ((value < setting->thresLow) && (setting->inactiveCount == 0)) {
    // The variable is below the low threshold
    setting->inactiveCount = inactiveWaitCycles;
    sendAlert(setting, value, false);
  }
}


// Send an alert message to IFTTT.com as an HTTP POST request.
// isOverHighThres = true means (value > thresHigh)
// isOverHighThres = false means (value < thresLow)
void sendAlert(ThresholdSetting_t * setting, int32_t value, bool isOverHighThres) {
  client.stop();
  if (client.connect("maker.ifttt.com", 80)) {
    client.println("POST /trigger/" IFTTT_EVENT_NAME "/with/key/" WEBHOOKS_KEY " HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Content-Type: application/json");

    sprintf(fieldBuffer,"The %s is too %s.", setting->variableName,
                        isOverHighThres ? "high" : "low");
    Serial.print("Sending new alert to IFTTT: ");
    Serial.println(fieldBuffer);

    sprintf(postBuffer,"{\"value1\":\"%s\",", fieldBuffer);

    sprintf(fieldBuffer,"\"value2\":\"The measurement was %" PRId32 " %s\"",
                        value, setting->measurementUnit);
    strcat(postBuffer, fieldBuffer);

    sprintf(fieldBuffer,",\"value3\":\"%s\"}",
                        isOverHighThres ? setting->adviceHigh : setting->adviceLow);
    strcat(postBuffer, fieldBuffer);

    size_t len = strlen(postBuffer);
    sprintf(fieldBuffer,"Content-Length: %u",len);
    client.println(fieldBuffer);
    client.println();
    client.print(postBuffer);
  }
  else {
    Serial.println("Client connection failed.");
  }
}
