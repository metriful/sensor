/* 
   IoT_cloud_logging.ino
   
   Example IoT data logging code for the Metriful MS430. 
   
   This example is designed for the following WiFi enabled hosts:
   * Arduino Nano 33 IoT
   * Arduino MKR WiFi 1010
   * NodeMCU ESP8266
   
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

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// How often to read and log data (every 3, 100, 300 seconds)
// Note: due to data rate limits on free cloud services, this should 
// be set to 100 or 300 seconds, not 3 seconds.
uint8_t cycle_period = CYCLE_PERIOD_100_S;

// The I2C address of the Metriful board
uint8_t i2c_7bit_address = I2C_ADDR_7BIT_SB_OPEN;

// Which particle sensor is attached (PPD42, SDS011, or OFF)
ParticleSensor_t particleSensor = SDS011;

// The details of the WiFi network to connect to:
char SSID[] = "PUT WIFI NETWORK NAME HERE IN QUOTES"; // network SSID (name)
char password[] = "PUT WIFI PASSWORD HERE IN QUOTES";     // network password

// IoT cloud settings
// This example uses the free IoT cloud hosting services provided 
// by Tago.io or Thingspeak.com
// Other free cloud providers are available.
// An account must have been set up with the relevant cloud provider 
// and a WiFi internet connection must exist. See the accompanying 
// readme and User Guide for more information.

// The chosen account's key/token must be put into the relevant define below.  
#define TAGO_DEVICE_TOKEN_STRING  "PASTE YOUR TOKEN HERE WITHIN QUOTES"
#define THINGSPEAK_API_KEY_STRING "PASTE YOUR API KEY HERE WITHIN QUOTES"

// Choose which provider to use
bool useTagoCloud = true;
// To use the ThingSpeak cloud, set: useTagoCloud=false

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

#if !defined(ARDUINO_SAMD_NANO_33_IOT) && !defined(ARDUINO_SAMD_MKRWIFI1010) && !defined(ESP8266)
#error ("This example program has been created for specific WiFi enabled hosts only.")
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

uint8_t transmit_buffer[1] = {0};


void setup() {
  // Initialize the host's pins, set up the serial port and reset:
  SensorHardwareSetup(i2c_7bit_address); 

  // Attempt to connect to the Wifi network:
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected.");
  
  ////////////////////////////////////////////////////////////////////
  
  // Apply chosen settings to the Metriful board
  if (particleSensor != OFF) {
    transmit_buffer[0] = particleSensor;
    TransmitI2C(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, transmit_buffer, 1);
  }
  transmit_buffer[0] = cycle_period;
  TransmitI2C(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, transmit_buffer, 1);

  // Enter cycle mode
  ready_assertion_event = false;
  TransmitI2C(i2c_7bit_address, CYCLE_MODE_CMD, 0, 0);
}


void loop() {

  // Wait for the next new data release, indicated by a falling edge on READY
  while (!ready_assertion_event) {
    yield();
  }
  ready_assertion_event = false;

  /* Read data from Metriful into the data structs. 
  For each category of data (air, sound, etc.) a pointer to the data struct is 
  passed to the ReceiveI2C() function. The received byte sequence fills the 
  struct in the correct order so that each field within the struct receives
  the value of an environmental quantity (temperature, sound level, etc.)
  */ 
  
  // Air data
  ReceiveI2C(i2c_7bit_address, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);
  
  /* Air quality data
  The initial self-calibration of the air quality data may take several
  minutes to complete. During this time the accuracy parameter is zero 
  and the data values are not valid.
  */ 
  ReceiveI2C(i2c_7bit_address, AIR_QUALITY_DATA_READ, (uint8_t *) &airQualityData, AIR_QUALITY_DATA_BYTES);
  
  // Light data
  ReceiveI2C(i2c_7bit_address, LIGHT_DATA_READ, (uint8_t *) &lightData, LIGHT_DATA_BYTES);
  
  // Sound data
  ReceiveI2C(i2c_7bit_address, SOUND_DATA_READ, (uint8_t *) &soundData, SOUND_DATA_BYTES);

  /* Particle data
  This requires the connection of a particulate sensor (invalid 
  values will be obtained if this sensor is not present).
  Also note that, due to the low pass filtering used, the 
  particle data become valid after an initial initialization 
  period of approximately one minute.
  */ 
  if (particleSensor != OFF) {
    ReceiveI2C(i2c_7bit_address, PARTICLE_DATA_READ, (uint8_t *) &particleData, PARTICLE_DATA_BYTES);
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi connection lost: attempting to reconnect.");
    WiFi.begin(SSID, password);
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
8 Particle concentration
  
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

    sprintf(fieldBuffer,",{\"variable\":\"particulates\",\"value\":%u.%02u}", 
        particleData.concentration_int, particleData.concentration_fr_2dp);
    strcat(postBuffer, fieldBuffer);
    
    sprintf(fieldBuffer,",{\"variable\":\"illuminance\",\"value\":%u.%02u}]", 
        lightData.illum_lux_int, lightData.illum_lux_fr_2dp);
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
    
    sprintf(fieldBuffer,"&field8=%u.%02u", particleData.concentration_int, 
                                           particleData.concentration_fr_2dp);
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
