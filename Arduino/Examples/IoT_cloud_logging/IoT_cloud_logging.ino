/* 
   IoT_cloud_logging.ino
   
   Example IoT data logging code for the Sense board. 
   
   This example is designed for the Arduino Nano 33 IoT only.
   Environmental data values are measured and logged to an internet 
   cloud account every 100 seconds. The example gives the choice of 
   using either the Tago.io or Thingspeak.com cloud – both of these 
   offer a free account for low data rates (compatible with this demo). 

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit https://github.com/metriful/sense
*/

#include <Metriful_Sense.h>
#include <stdint.h>
#include <SPI.h>
#include <WiFiNINA.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// How often to read and log data (every 3, 100, 300 seconds)
// Note that due to data rate limits on free cloud services, this should 
// be set to 100 or 300 seconds, not 3 seconds.
uint8_t cycle_period = CYCLE_PERIOD_100_S;

// The I2C address of the Sense board
uint8_t i2c_7bit_address = I2C_ADDR_7BIT_SB_OPEN;

// Whether to read the particle data (set false if no PPD42 particle 
// sensor is connected, to avoid seeing spurious data).
bool getParticleData = true;

// Whether to print data over the serial port as
// well as sending to the IoT cloud
bool printSerialData = true;

// If printSerialData = true, how to print the data over the serial port. 
// If printDataAsColumns = true, data are columns of numbers, useful for 
// transferring to a spreadsheet application. Otherwise, data are printed 
// with explanatory labels and units.
bool printDataAsColumns = false;

// The SSID (name) and password for the WiFi network that 
// the Arduino will connect to.
char ssid[] = "PUT WIFI NETWORK NAME HERE IN QUOTES"; // network SSID (name)
char pass[] = "PUT WIFI PASSWORD HERE IN QUOTES";     // network password

// IoT cloud settings
// This example demonstrates use of the free IoT cloud hosting 
// services provided by Tago.io or Thingspeak.com
// Other free cloud providers are available.
// An account must have been set up with the relevant cloud provider 
// and a WiFi internet connection must exist. See the accompanying readme 
// and User Guide for more information.

// The chosen account's key/token must be put into the relevant define below.  
#define TAGO_DEVICE_TOKEN_STRING  "PASTE YOUR TOKEN HERE WITHIN QUOTES"
#define THINGSPEAK_API_KEY_STRING "PASTE YOUR API KEY HERE WITHIN QUOTES"

// Choose which provider to use
bool useTagoCloud = true;
// To use the ThingSpeak cloud, set: useTagoCloud=false

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

#ifndef ARDUINO_SAMD_NANO_33_IOT
#error ("This example program has been created specifically for the Arduino Nano 33 IoT.")
#endif

WiFiClient client;

// Buffers for assembling http POST requests
char postBuffer[450] = {0};
char fieldBuffer[50] = {0};

// Structs for data
AirData_t airData = {0};
AirQualityData_t airQualityData = {0};
LightData_t lightData = {0}; 
ParticleData_t particleData = {0};
SoundData_t soundData = {0};

// Buffer for commands (big enough to fit the largest send transaction):
uint8_t transmit_buffer[LIGHT_INTERRUPT_THRESHOLD_BYTES] = {0};


void setup() {
  // Initialize the Arduino pins, set up the serial port and reset:
  SenseHardwareSetup(i2c_7bit_address); 

  // attempt to connect to the Wifi network:
  int32_t status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");Serial.println(ssid);
    // Connect to WPA/WPA2 network.
    status = WiFi.begin(ssid, pass);
    delay(3000);
  }
  Serial.println("Connected.");
  // print the SSID of the network
  Serial.print("SSID: ");Serial.println(WiFi.SSID());

  // print WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");Serial.println(ip);

  // print the received signal strength:
  int32_t rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);Serial.println(" dBm");
  
  ////////////////////////////////////////////////////////////////////
  
  // Apply chosen settings to the Sense board
  if (getParticleData) {
    transmit_buffer[0] = ENABLED;
    TransmitI2C(i2c_7bit_address, PARTICLE_SENSOR_ENABLE_REG, transmit_buffer, 1);
  }
  transmit_buffer[0] = cycle_period;
  TransmitI2C(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, transmit_buffer, 1);

  Serial.println("Entering cycle mode and waiting for data.");
  ready_assertion_event = false;
  TransmitI2C(i2c_7bit_address, CYCLE_MODE_CMD, transmit_buffer, 0);
}


void loop() {
  // Wait for the next new data release, indicated by a falling edge on READY
  while (!ready_assertion_event) {}
  ready_assertion_event = false;

  /* Read data from Sense into the data structs. 
  For each category of data (air, sound, etc.) a pointer to the data struct is 
  passed to the ReceiveI2C() function. The received byte sequence fills the 
  struct in the correct order so that each field within the struct receives
  the value of an environmental quantity (temperature, sound level, etc.)
  */ 
  
  // Air data
  ReceiveI2C(i2c_7bit_address, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);
  
  /* Air quality data
  Note that the initial self-calibration of the air quality data 
  takes a few minutes to complete. During this time the accuracy 
  parameter is zero and the data values do not change.
  */ 
  ReceiveI2C(i2c_7bit_address, AIR_QUALITY_DATA_READ, (uint8_t *) &airQualityData, AIR_QUALITY_DATA_BYTES);
  
  // Light data
  ReceiveI2C(i2c_7bit_address, LIGHT_DATA_READ, (uint8_t *) &lightData, LIGHT_DATA_BYTES);
  
  // Sound data
  ReceiveI2C(i2c_7bit_address, SOUND_DATA_READ, (uint8_t *) &soundData, SOUND_DATA_BYTES);
  
  /* Particle data
  Note that this requires the connection of a PPD42 particle 
  sensor (invalid values will be obtained if this sensor is not present).
  Also note that, due to the low pass filtering used, the 
  particle data become valid after an initial stabilization 
  period of approximately two minutes.
  */ 
  if (getParticleData) {
    ReceiveI2C(i2c_7bit_address, PARTICLE_DATA_READ, (uint8_t *) &particleData, PARTICLE_DATA_BYTES);
  }

  if (printSerialData) {
    // Print all data to the serial port
    printAirData(&airData, printDataAsColumns);
    printAirQualityData(&airQualityData, printDataAsColumns);
    printLightData(&lightData, printDataAsColumns);
    printSoundData(&soundData, printDataAsColumns);
    if (getParticleData) {
      printParticleData(&particleData, printDataAsColumns);
    }
    Serial.println();
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi connection lost: attempting to reconnect.");
    WiFi.begin(ssid, pass);
  }
  else {
    if (useTagoCloud) {
      http_POST_data_Tago_cloud();
    }
    else {
      http_POST_data_Thingspeak_cloud();
    }
  }
}

/* For both example cloud providers, the following quantities will be sent:
1 Temperature/C
2 Pressure/Pa
3 Humidity/%
4 Air quality index
5 bVOC/ppm
6 SPL/dBA
7 Illuminance/lux
8 Particle concentration/ppL
  
   Additionally, for Tago, the following is sent:
9  Air Quality Assessment summary (Good, Bad, etc.) 
10 Peak sound amplitude / mPa
*/ 

// Assemble the data into the required format, then send it to the
// Tago.io cloud as an HTTP POST request.
void http_POST_data_Tago_cloud(void) {
  if (client.connect("api.tago.io", 80)) { 
    client.println("POST /data HTTP/1.1"); 
    client.println("Content-Type: application/json");
    client.println("Device-Token: " TAGO_DEVICE_TOKEN_STRING);

    uint8_t T_positive_integer = airData.T_C_int_with_sign & TEMPERATURE_VALUE_MASK;
    // If the most-significant bit is set, the temperature is negative (below 0 C)
    if ((airData.T_C_int_with_sign & TEMPERATURE_SIGN_MASK) != 0) {
      // The bit is set: celsius temperature is negative
      sprintf(postBuffer,"[{\"variable\":\"temperature\",\"value\":-%u.%u}", 
          T_positive_integer, airData.T_C_fr_1dp);
    }
    else {
      // The bit is not set: celsius temperature is positive
      sprintf(postBuffer,"[{\"variable\":\"temperature\",\"value\":%u.%u}", 
          T_positive_integer, airData.T_C_fr_1dp);
    }
    
    sprintf(fieldBuffer,",{\"variable\":\"pressure\",\"value\":%lu}", airData.P_Pa);
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
    
    sprintf(fieldBuffer,",{\"variable\":\"illuminance\",\"value\":%u.%02u}", 
        lightData.illum_lux_int, lightData.illum_lux_fr_2dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,",{\"variable\":\"particulates\",\"value\":%u}]", 
        particleData.concentration_ppL);
    strcat(postBuffer, fieldBuffer);
    
    int len = strlen(postBuffer);
    sprintf(fieldBuffer,"Content-Length: %u",len);  
    client.println(fieldBuffer); 
    client.println(); 
    client.println(postBuffer);
  }
  else {
    Serial.println("Connection failed");
  }
}

// Assemble the data into the required format, then send it to the
// Thingspeak.com cloud as an HTTP POST request.
void http_POST_data_Thingspeak_cloud(void) {
  if (client.connect("api.thingspeak.com", 80)) { 
    client.println("POST /update HTTP/1.1"); 
    client.println("Content-Type: application/x-www-form-urlencoded");
    
    uint8_t T_positive_integer = airData.T_C_int_with_sign & TEMPERATURE_VALUE_MASK;
    // If the most-significant bit is set, the temperature is negative (below 0 C)
    if ((airData.T_C_int_with_sign & TEMPERATURE_SIGN_MASK) != 0) {
      // The bit is set: celsius temperature is negative
      sprintf(postBuffer,"api_key=" THINGSPEAK_API_KEY_STRING "&field1=-%u.%u", 
          T_positive_integer, airData.T_C_fr_1dp);
    }
    else {
      // The bit is not set: celsius temperature is positive
      sprintf(postBuffer,"api_key=" THINGSPEAK_API_KEY_STRING "&field1=%u.%u", 
          T_positive_integer, airData.T_C_fr_1dp);
    }
    
    sprintf(fieldBuffer,"&field2=%lu", airData.P_Pa);
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
    
    sprintf(fieldBuffer,"&field8=%u", particleData.concentration_ppL);
    strcat(postBuffer, fieldBuffer);
    
    int len = strlen(postBuffer);
    sprintf(fieldBuffer,"Content-Length: %u",len);  
    client.println(fieldBuffer); 
    client.println(); 
    client.println(postBuffer);
  }
  else {
    Serial.println("Connection failed");
  }
}
