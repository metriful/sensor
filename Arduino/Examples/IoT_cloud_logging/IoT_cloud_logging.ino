/* 
   IoT_cloud_logging.ino
   
   Example IoT data logging code for the Metriful MS430. 
   
   This example is designed for the following WiFi enabled hosts:
   * Arduino Nano 33 IoT
   * Arduino MKR WiFi 1010
   * ESP8266 boards (e.g. Wemos D1, NodeMCU)
   * ESP32 boards (e.g. DOIT DevKit v1)
   
   Environmental data values are measured and logged to an internet 
   cloud account every 100 seconds, using a WiFi network. The example 
   gives the choice of using either the Tago.io or Thingspeak.com 
   clouds – both of these offer a free account for low data rates.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>
#include <WiFi_functions.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// How often to read and log data (every 100 or 300 seconds)
// Note: due to data rate limits on free cloud services, this should 
// be set to 100 or 300 seconds, not 3 seconds.
uint8_t cycle_period = CYCLE_PERIOD_100_S;

// The details of the WiFi network:
char SSID[] = "PUT WIFI NETWORK NAME HERE IN QUOTES"; // network SSID (name)
char password[] = "PUT WIFI PASSWORD HERE IN QUOTES"; // network password

// IoT cloud settings
// This example uses the free IoT cloud hosting services provided 
// by Tago.io or Thingspeak.com
// Other free cloud providers are available.
// An account must have been set up with the relevant cloud provider 
// and a WiFi internet connection must exist. See the accompanying 
// readme and User Guide for more information.

// The chosen account's key/token must be put into the relevant define below.  
#define TAGO_DEVICE_TOKEN_STRING "PASTE YOUR TOKEN HERE WITHIN QUOTES"
#define THINGSPEAK_API_KEY_STRING "PASTE YOUR API KEY HERE WITHIN QUOTES"

// Choose which provider to use
bool useTagoCloud = true;
// To use the ThingSpeak cloud, set: useTagoCloud=false

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

#if !defined(HAS_WIFI)
#error ("This example program has been created for specific WiFi enabled hosts only.")
#endif

WiFiClient client;

// Buffers for assembling http POST requests
char postBuffer[450] = {0};
char fieldBuffer[70] = {0};

// Structs for data
AirData_t airData = {0};
AirQualityData_t airQualityData = {0};
LightData_t lightData = {0}; 
ParticleData_t particleData = {0};
SoundData_t soundData = {0};

void setup() {
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


void loop() {

  // Wait for the next new data release, indicated by a falling edge on READY
  while (!ready_assertion_event) {
    yield();
  }
  ready_assertion_event = false;

  /* Read data from the MS430 into the data structs. 
  For each category of data (air, sound, etc.) a pointer to the data struct is 
  passed to the ReceiveI2C() function. The received byte sequence fills the 
  struct in the correct order so that each field within the struct receives
  the value of an environmental quantity (temperature, sound level, etc.)
  */ 
  
  // Air data
  // Choose output temperature unit (C or F) in Metriful_sensor.h
  ReceiveI2C(I2C_ADDRESS, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);
  
  /* Air quality data
  The initial self-calibration of the air quality data may take several
  minutes to complete. During this time the accuracy parameter is zero 
  and the data values are not valid.
  */ 
  ReceiveI2C(I2C_ADDRESS, AIR_QUALITY_DATA_READ, (uint8_t *) &airQualityData, AIR_QUALITY_DATA_BYTES);
  
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
  if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF) {
    ReceiveI2C(I2C_ADDRESS, PARTICLE_DATA_READ, (uint8_t *) &particleData, PARTICLE_DATA_BYTES);
  }

  // Check that WiFi is still connected
  uint8_t wifiStatus = WiFi.status();
  if (wifiStatus != WL_CONNECTED) {
    // There is a problem with the WiFi connection: attempt to reconnect.
    Serial.print("Wifi status: ");
    Serial.println(interpret_WiFi_status(wifiStatus));
    connectToWiFi(SSID, password);
    ready_assertion_event = false;
  }

  // Send data to the cloud
  if (useTagoCloud) {
    http_POST_data_Tago_cloud();
  }
  else {
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
  
   Additionally, for Tago, the following is sent:
9  Air Quality Assessment summary (Good, Bad, etc.) 
10 Peak sound amplitude / mPa
*/ 

// Assemble the data into the required format, then send it to the
// Tago.io cloud as an HTTP POST request.
void http_POST_data_Tago_cloud(void) {
  client.stop();
  if (client.connect("api.tago.io", 80)) {
    client.println("POST /data HTTP/1.1");
    client.println("Host: api.tago.io");
    client.println("Content-Type: application/json");
    client.println("Device-Token: " TAGO_DEVICE_TOKEN_STRING);
    
    uint8_t T_intPart = 0;
    uint8_t T_fractionalPart = 0;
    bool isPositive = true;
    getTemperature(&airData, &T_intPart, &T_fractionalPart, &isPositive);
    sprintf(postBuffer,"[{\"variable\":\"temperature\",\"value\":%s%u.%u}",
                         isPositive?"":"-", T_intPart, T_fractionalPart);
    
    sprintf(fieldBuffer,",{\"variable\":\"pressure\",\"value\":%" PRIu32 "}", airData.P_Pa);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,",{\"variable\":\"humidity\",\"value\":%u.%u}", 
            airData.H_pc_int, airData.H_pc_fr_1dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,",{\"variable\":\"aqi\",\"value\":%u.%u}", 
            airQualityData.AQI_int, airQualityData.AQI_fr_1dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,",{\"variable\":\"aqi_string\",\"value\":\"%s\"}", 
            interpret_AQI_value(airQualityData.AQI_int));
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,",{\"variable\":\"bvoc\",\"value\":%u.%02u}", 
            airQualityData.bVOC_int, airQualityData.bVOC_fr_2dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,",{\"variable\":\"spl\",\"value\":%u.%u}", 
            soundData.SPL_dBA_int, soundData.SPL_dBA_fr_1dp);
    strcat(postBuffer, fieldBuffer);

    sprintf(fieldBuffer,",{\"variable\":\"peak_amp\",\"value\":%u.%02u}", 
            soundData.peak_amp_mPa_int, soundData.peak_amp_mPa_fr_2dp);
    strcat(postBuffer, fieldBuffer);

    sprintf(fieldBuffer,",{\"variable\":\"particulates\",\"value\":%u.%02u}", 
            particleData.concentration_int, particleData.concentration_fr_2dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,",{\"variable\":\"illuminance\",\"value\":%u.%02u}]", 
            lightData.illum_lux_int, lightData.illum_lux_fr_2dp);
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


// Assemble the data into the required format, then send it to the
// Thingspeak.com cloud as an HTTP POST request.
void http_POST_data_Thingspeak_cloud(void) {
  client.stop();
  if (client.connect("api.thingspeak.com", 80)) { 
    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    
    strcpy(postBuffer,"api_key=" THINGSPEAK_API_KEY_STRING);
    
    uint8_t T_intPart = 0;
    uint8_t T_fractionalPart = 0;
    bool isPositive = true;
    getTemperature(&airData, &T_intPart, &T_fractionalPart, &isPositive);
    sprintf(fieldBuffer,"&field1=%s%u.%u", isPositive?"":"-", T_intPart, T_fractionalPart);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,"&field2=%" PRIu32, airData.P_Pa);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,"&field3=%u.%u", airData.H_pc_int, airData.H_pc_fr_1dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,"&field4=%u.%u", airQualityData.AQI_int, airQualityData.AQI_fr_1dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,"&field5=%u.%02u", airQualityData.bVOC_int, airQualityData.bVOC_fr_2dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,"&field6=%u.%u", soundData.SPL_dBA_int, soundData.SPL_dBA_fr_1dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,"&field7=%u.%02u", lightData.illum_lux_int, lightData.illum_lux_fr_2dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,"&field8=%u.%02u", particleData.concentration_int, 
                                           particleData.concentration_fr_2dp);
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
