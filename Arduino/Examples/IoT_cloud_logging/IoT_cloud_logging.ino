/* 
   IoT_cloud_logging.ino

   Example IoT data logging code for the Metriful MS430.

   This example is designed for the following WiFi enabled hosts:
   * Arduino Nano 33 IoT
   * Arduino MKR WiFi 1010
   * ESP8266 boards (e.g. Wemos D1, NodeMCU)
   * ESP32 boards (e.g. DOIT DevKit v1)
   * Raspberry Pi Pico W

   Environmental data values are measured and logged to an internet
   cloud account every 100 seconds, using a WiFi network. The example
   gives the choice of using either the Tago.io or Thingspeak.com
   clouds – both of these offer a free account for low data rates.

   Copyright 2020-2023 Metriful Ltd.
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>
#include <WiFi_functions.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// How often to read and log data (every 100 or 300 seconds)
// Note: due to data rate limits on free cloud services, this should 
// be set to 100 or 300 seconds, not 3 seconds.
uint8_t cycle_period = CYCLE_PERIOD_100_S;

// The details of the WiFi network that we will connect to:
const char * SSID = "PUT WIFI NETWORK NAME HERE"; // network SSID (name)
const char * password = "PUT WIFI PASSWORD HERE";

// IoT cloud settings
// This example uses the free IoT cloud hosting services provided 
// by Tago.io or Thingspeak.com
// An account must have been set up with the relevant cloud provider 
// and a WiFi internet connection must exist. See the readme for
// more information.

// Choose which provider to use
bool useTagoCloud = true;
// To use the ThingSpeak cloud, set: useTagoCloud=false

// The chosen account's key/token must be put into the relevant define below.  
#define TAGO_DEVICE_TOKEN_STRING "PASTE YOUR TOKEN HERE WITHIN QUOTES"
// or
#define THINGSPEAK_API_KEY_STRING "PASTE YOUR API KEY HERE WITHIN QUOTES"

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

#if !defined(HAS_WIFI)
#error ("This example program has been created for specific WiFi enabled hosts only.")
#endif

WiFiClient client;

// Buffers for assembling http POST requests
char postBuffer[600] = {0};
char fieldBuffer[70] = {0};
char valueBuffer[20] = {0};

typedef enum {FIRST, LAST, OTHER} FirstLast;

// Structs for data
AirData_t airData = {0};
AirQualityData_t airQualityData = {0};
LightData_t lightData = {0}; 
ParticleData_t particleData = {0};
SoundData_t soundData = {0};

void setup()
{
  // Initialize the host's pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 

  connectToWiFi(SSID, password);
  
  // Apply chosen settings to the MS430
  uint8_t particleSensor = PARTICLE_SENSOR;
  TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, &particleSensor, 1);
  TransmitI2C(I2C_ADDRESS, CYCLE_TIME_PERIOD_REG, &cycle_period, 1);

  // Enter cycle mode
  ready_assertion_event = false;
  TransmitI2C(I2C_ADDRESS, CYCLE_MODE_CMD, 0, 0);
}


void loop()
{
  // Wait for the next new data release, indicated by a falling edge on READY
  while (!ready_assertion_event)
  {
    yield();
  }
  ready_assertion_event = false;

  // Read data from the MS430 into the data structs. 
  
  // Air data
  // You can enable Fahrenheit temperature unit in Metriful_sensor.h
  ReceiveI2C(I2C_ADDRESS, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);
  
  /* Air quality data
  The initial self-calibration of the air quality data may take several
  minutes to complete. During this time the accuracy parameter is zero 
  and the data values are not valid.
  */ 
  ReceiveI2C(I2C_ADDRESS, AIR_QUALITY_DATA_READ, (uint8_t *) &airQualityData,
             AIR_QUALITY_DATA_BYTES);
  
  // Light data
  ReceiveI2C(I2C_ADDRESS, LIGHT_DATA_READ, (uint8_t *) &lightData, LIGHT_DATA_BYTES);
  
  // Sound data
  ReceiveI2C(I2C_ADDRESS, SOUND_DATA_READ, (uint8_t *) &soundData, SOUND_DATA_BYTES);

  /* Particle data
  This requires the connection of a particulate sensor (invalid 
  values will be obtained if this sensor is not present).
  Specify your sensor model (PPD42 or SDS011) in Metriful_sensor.h
  Also note that, due to the low pass filtering used, the 
  particle data become valid after an initial initialization 
  period of approximately one minute.
  */ 
  if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF)
  {
    ReceiveI2C(I2C_ADDRESS, PARTICLE_DATA_READ, (uint8_t *) &particleData, PARTICLE_DATA_BYTES);
  }

  // Check that WiFi is still connected
  uint8_t wifiStatus = WiFi.status();
  if (wifiStatus != WL_CONNECTED)
  {
    // There is a problem with the WiFi connection: attempt to reconnect.
    Serial.print("Wifi status: ");
    Serial.println(interpret_WiFi_status(wifiStatus));
    connectToWiFi(SSID, password);
    ready_assertion_event = false;
  }

  // Send data to the cloud
  if (useTagoCloud)
  {
    http_POST_data_Tago_cloud();
  }
  else
  {
    http_POST_data_Thingspeak_cloud();
  }
}


/* For both example cloud providers, the following quantities will be sent:
1 Temperature (C or F)
2 Pressure/Pa
3 Humidity/%
4 Air quality index
5 bVOC/ppm
6 SPL/dBA
7 Illuminance/lux
8 Particle concentration
  
   Additionally, for Tago, the following are sent:
9  Air Quality Assessment summary (Good, Bad, etc.) 
10 Peak sound amplitude / mPa
*/ 

// Add the field for a single data variable to the Tago HTTP response in
// the postBuffer.
void addTagoVariable(FirstLast firstLast, const char * variableName,
                     const char * valueFormat, ...)
{
  va_list args;
  va_start(args, valueFormat);
  vsnprintf(valueBuffer, sizeof valueBuffer, valueFormat, args);
  va_end(args);
  const char * fieldFormat;
  switch (firstLast)
  {
    case FIRST:
        postBuffer[0] = 0;
        fieldFormat = "[{\"variable\":\"%s\",\"value\":%s},";
        break;
    case LAST:
        fieldFormat = "{\"variable\":\"%s\",\"value\":%s}]";
        break;
    case OTHER: default:
        fieldFormat = "{\"variable\":\"%s\",\"value\":%s},";
  }
  snprintf(fieldBuffer, sizeof fieldBuffer, fieldFormat, variableName, valueBuffer);
  strncat(postBuffer, fieldBuffer, (sizeof postBuffer) - strlen(postBuffer) - 1);
}

// Assemble the data into the required format, then send it to the
// Tago.io cloud as an HTTP POST request.
void http_POST_data_Tago_cloud(void)
{
  client.stop();
  if (client.connect("api.tago.io", 80))
  {
    client.println("POST /data HTTP/1.1");
    client.println("Host: api.tago.io");
    client.println("Content-Type: application/json");
    client.println("Device-Token: " TAGO_DEVICE_TOKEN_STRING);

    uint8_t T_intPart = 0;
    uint8_t T_fractionalPart = 0;
    bool isPositive = true;
    getTemperature(&airData, &T_intPart, &T_fractionalPart, &isPositive);

    addTagoVariable(FIRST, "temperature", "%s%u.%u", isPositive ? "" : "-", T_intPart,
                    T_fractionalPart);
    addTagoVariable(OTHER, "pressure", "%" PRIu32, airData.P_Pa);
    addTagoVariable(OTHER, "humidity", "%u.%u", airData.H_pc_int, airData.H_pc_fr_1dp);
    addTagoVariable(OTHER, "aqi", "%u.%u", airQualityData.AQI_int, airQualityData.AQI_fr_1dp);
    addTagoVariable(OTHER, "aqi_string", "\"%s\"", interpret_AQI_value(airQualityData.AQI_int));
    addTagoVariable(OTHER, "bvoc", "%u.%02u", airQualityData.bVOC_int, airQualityData.bVOC_fr_2dp);
    addTagoVariable(OTHER, "spl", "%u.%u", soundData.SPL_dBA_int, soundData.SPL_dBA_fr_1dp);
    addTagoVariable(OTHER, "peak_amp", "%u.%02u", soundData.peak_amp_mPa_int,
                    soundData.peak_amp_mPa_fr_2dp);
    addTagoVariable(OTHER, "particulates", "%u.%02u", particleData.concentration_int,
                    particleData.concentration_fr_2dp);
    addTagoVariable(LAST, "illuminance", "%u.%02u", lightData.illum_lux_int,
                    lightData.illum_lux_fr_2dp);

    snprintf(fieldBuffer, sizeof fieldBuffer, "Content-Length: %u", strlen(postBuffer));
    client.println(fieldBuffer);
    client.println();
    client.print(postBuffer);
  }
  else
  {
    Serial.println("Client connection failed.");
  }
}

// Add the field for a single data variable to the Thingspeak HTTP
// response in the postBuffer.
void addThingspeakVariable(uint8_t fieldNumber, const char * valueFormat, ...)
{
  va_list args;
  va_start(args, valueFormat);
  vsnprintf(valueBuffer, sizeof valueBuffer, valueFormat, args);
  va_end(args);
  snprintf(fieldBuffer, sizeof fieldBuffer, "&field%u=%s", fieldNumber, valueBuffer);
  strncat(postBuffer, fieldBuffer, (sizeof postBuffer) - strlen(postBuffer) - 1);
}

// Assemble the data into the required format, then send it to the
// Thingspeak.com cloud as an HTTP POST request.
void http_POST_data_Thingspeak_cloud(void)
{
  client.stop();
  if (client.connect("api.thingspeak.com", 80))
  { 
    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Content-Type: application/x-www-form-urlencoded");

    uint8_t T_intPart = 0;
    uint8_t T_fractionalPart = 0;
    bool isPositive = true;
    getTemperature(&airData, &T_intPart, &T_fractionalPart, &isPositive);
    
    snprintf(postBuffer, sizeof postBuffer, "%s", "api_key=" THINGSPEAK_API_KEY_STRING);
    addThingspeakVariable(1, "%s%u.%u", isPositive ? "" : "-", T_intPart, T_fractionalPart);
    addThingspeakVariable(2, "%" PRIu32, airData.P_Pa);
    addThingspeakVariable(3, "%u.%u", airData.H_pc_int, airData.H_pc_fr_1dp);
    addThingspeakVariable(4, "%u.%u", airQualityData.AQI_int, airQualityData.AQI_fr_1dp);
    addThingspeakVariable(5, "%u.%02u", airQualityData.bVOC_int, airQualityData.bVOC_fr_2dp);
    addThingspeakVariable(6, "%u.%u", soundData.SPL_dBA_int, soundData.SPL_dBA_fr_1dp);
    addThingspeakVariable(7, "%u.%02u", lightData.illum_lux_int, lightData.illum_lux_fr_2dp);
    addThingspeakVariable(8, "%u.%02u", particleData.concentration_int,
                          particleData.concentration_fr_2dp);
    snprintf(fieldBuffer, sizeof fieldBuffer, "Content-Length: %u", strlen(postBuffer));
    client.println(fieldBuffer); 
    client.println(); 
    client.print(postBuffer);
  }
  else
  {
    Serial.println("Client connection failed.");
  }
}
