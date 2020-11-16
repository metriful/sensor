/* 
   Home_Assistant.ino
   
   Example code for sending environment data from the Metriful MS430 to
   an installation of Home Assistant on your local WiFi network.
   For more information, visit www.home-assistant.io

   This example is designed for the following WiFi enabled hosts:
   * Arduino Nano 33 IoT
   * Arduino MKR WiFi 1010
   * ESP8266 boards (e.g. Wemos D1, NodeMCU)
   * ESP32 boards (e.g. DOIT DevKit v1)
   
   Data are sent at regular intervals over your WiFi network to Home 
   Assistant and can be viewed on the dashboard or used to control 
   home automation tasks. More setup information is provided in the 
   Readme and User Guide.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>
#include <WiFi_functions.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// How often to read and report the data (every 3, 100 or 300 seconds)
uint8_t cycle_period = CYCLE_PERIOD_100_S;

// The details of the WiFi network:
char SSID[] = "PUT WIFI NETWORK NAME HERE IN QUOTES"; // network SSID (name)
char password[] = "PUT WIFI PASSWORD HERE IN QUOTES"; // network password

// Home Assistant settings

// You must have already installed Home Assistant on a computer on your 
// network. Go to www.home-assistant.io for help on this.

// Choose a unique name for this MS430 sensor board so you can identify it.
// Variables in HA will have names like: SENSOR_NAME.temperature, etc.
#define SENSOR_NAME "kitchen3"

// Change this to the IP address of the computer running Home Assistant. 
// You can find this from the admin interface of your router.
#define HOME_ASSISTANT_IP "192.168.43.144"

// Security access token: the Readme and User Guide explain how to get this
#define LONG_LIVED_ACCESS_TOKEN "PASTE YOUR TOKEN HERE WITHIN QUOTES"

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

// Define the display attributes of data sent to Home Assistant. 
// The chosen name, unit and icon will appear in on the overview 
// dashboard in Home Assistant. The icons can be chosen from 
// https://cdn.materialdesignicons.com/5.3.45/    
// (remove the "mdi-" part from the icon name).
// The attribute fields are: {name, unit, icon, decimal places}
HA_Attributes_t pressure = {"Pressure","Pa","weather-cloudy",0};
HA_Attributes_t humidity = {"Humidity","%","water-percent",1};
HA_Attributes_t illuminance = {"Illuminance","lx","white-balance-sunny",2};
HA_Attributes_t soundLevel = {"Sound level","dBA","microphone",1};
HA_Attributes_t peakAmplitude = {"Sound peak","mPa","waveform",2};
HA_Attributes_t AQI = {"Air Quality Index"," ","thought-bubble-outline",1};
HA_Attributes_t AQ_assessment = {"Air quality assessment","","flower-tulip",0};
#if (PARTICLE_SENSOR == PARTICLE_SENSOR_PPD42)
  HA_Attributes_t particulates = {"Particle concentration","ppL","chart-bubble",0};
#else
  HA_Attributes_t particulates = {"Particle concentration",SDS011_UNIT_SYMBOL,"chart-bubble",2};
#endif
#ifdef USE_FAHRENHEIT
  HA_Attributes_t temperature = {"Temperature",FAHRENHEIT_SYMBOL,"thermometer",1};
#else
  HA_Attributes_t temperature = {"Temperature",CELSIUS_SYMBOL,"thermometer",1};
#endif


void setup() {
  // Initialize the host's pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 

  connectToWiFi(SSID, password);
  
  // Apply settings to the MS430 and enter cycle mode
  uint8_t particleSensorCode = PARTICLE_SENSOR;
  TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, &particleSensorCode, 1);
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

  // Read data from the MS430 into the data structs. 
  ReceiveI2C(I2C_ADDRESS, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);
  ReceiveI2C(I2C_ADDRESS, AIR_QUALITY_DATA_READ, (uint8_t *) &airQualityData, AIR_QUALITY_DATA_BYTES);
  ReceiveI2C(I2C_ADDRESS, LIGHT_DATA_READ, (uint8_t *) &lightData, LIGHT_DATA_BYTES);
  ReceiveI2C(I2C_ADDRESS, SOUND_DATA_READ, (uint8_t *) &soundData, SOUND_DATA_BYTES);
  ReceiveI2C(I2C_ADDRESS, PARTICLE_DATA_READ, (uint8_t *) &particleData, PARTICLE_DATA_BYTES);

  // Check that WiFi is still connected
  uint8_t wifiStatus = WiFi.status();
  if (wifiStatus != WL_CONNECTED) {
    // There is a problem with the WiFi connection: attempt to reconnect.
    Serial.print("Wifi status: ");
    Serial.println(interpret_WiFi_status(wifiStatus));
    connectToWiFi(SSID, password);
    ready_assertion_event = false;
  }
  
  uint8_t T_intPart = 0;
  uint8_t T_fractionalPart = 0;
  bool isPositive = true;
  getTemperature(&airData, &T_intPart, &T_fractionalPart, &isPositive);
  
  // Send data to Home Assistant
  sendNumericData(&temperature, (uint32_t) T_intPart, T_fractionalPart, isPositive);
  sendNumericData(&pressure, (uint32_t) airData.P_Pa, 0, true);
  sendNumericData(&humidity, (uint32_t) airData.H_pc_int, airData.H_pc_fr_1dp, true);
  sendNumericData(&illuminance, (uint32_t) lightData.illum_lux_int, lightData.illum_lux_fr_2dp, true);
  sendNumericData(&soundLevel, (uint32_t) soundData.SPL_dBA_int, soundData.SPL_dBA_fr_1dp, true);
  sendNumericData(&peakAmplitude, (uint32_t) soundData.peak_amp_mPa_int, 
                  soundData.peak_amp_mPa_fr_2dp, true);
  sendNumericData(&AQI, (uint32_t) airQualityData.AQI_int, airQualityData.AQI_fr_1dp, true);
  if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF) {
    sendNumericData(&particulates, (uint32_t) particleData.concentration_int, 
                    particleData.concentration_fr_2dp, true);
  }
  sendTextData(&AQ_assessment, interpret_AQI_value(airQualityData.AQI_int));
}

// Send numeric data with specified sign, integer and fractional parts
void sendNumericData(const HA_Attributes_t * attributes, uint32_t valueInteger, 
                             uint8_t valueDecimal, bool isPositive) {
  char valueText[20] = {0};
  const char * sign = isPositive ? "" : "-";
  switch (attributes->decimalPlaces) {
    case 0:
    default:
      sprintf(valueText,"%s%" PRIu32, sign, valueInteger);
      break;
    case 1:
      sprintf(valueText,"%s%" PRIu32 ".%u", sign, valueInteger, valueDecimal);
      break;
    case 2:
      sprintf(valueText,"%s%" PRIu32 ".%02u", sign, valueInteger, valueDecimal);
      break;
  }
  http_POST_Home_Assistant(attributes, valueText);
}

// Send a text string: must have quotation marks added
void sendTextData(const HA_Attributes_t * attributes, const char * valueText) {
  char quotedText[20] = {0};
  sprintf(quotedText,"\"%s\"", valueText);
  http_POST_Home_Assistant(attributes, quotedText);
}

// Send the data to Home Assistant as an HTTP POST request.
void http_POST_Home_Assistant(const HA_Attributes_t * attributes, const char * valueText) {
  client.stop();
  if (client.connect(HOME_ASSISTANT_IP, 8123)) {
    // Form the URL from the name but replace spaces with underscores
    strcpy(fieldBuffer,attributes->name);
    for (uint8_t i=0; i<strlen(fieldBuffer); i++) {
      if (fieldBuffer[i] == ' ') {
        fieldBuffer[i] = '_';
      }
    }
    sprintf(postBuffer,"POST /api/states/" SENSOR_NAME ".%s HTTP/1.1", fieldBuffer);
    client.println(postBuffer);
    client.println("Host: " HOME_ASSISTANT_IP ":8123");
    client.println("Content-Type: application/json");
    client.println("Authorization: Bearer " LONG_LIVED_ACCESS_TOKEN);
    
    // Assemble the JSON content string:
    sprintf(postBuffer,"{\"state\":%s,\"attributes\":{\"unit_of_measurement\""
                       ":\"%s\",\"friendly_name\":\"%s\",\"icon\":\"mdi:%s\"}}",
                       valueText, attributes->unit, attributes->name, attributes->icon);
    
    sprintf(fieldBuffer,"Content-Length: %u", strlen(postBuffer));  
    client.println(fieldBuffer);
    client.println();
    client.print(postBuffer);
  }
  else {
    Serial.println("Client connection failed.");
  }
}
