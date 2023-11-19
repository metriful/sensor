/* 
   graph_web_server.ino

   Serve a web page over a WiFi network, displaying graphs showing
   environment data read from the Metriful MS430. A CSV data file is
   also downloadable from the page.

   This example is designed for the following WiFi enabled hosts:
   * Arduino Nano 33 IoT
   * Arduino MKR WiFi 1010
   * ESP8266 boards (e.g. Wemos D1, NodeMCU)
   * ESP32 boards (e.g. DOIT DevKit v1)
   * Raspberry Pi Pico W

   The host can either connect to an existing WiFi network, or generate
   its own for other devices to connect to (Access Point mode).

   The browser which views the web page uses the Plotly javascript
   library to generate the graphs. This is automatically downloaded
   over the internet, or can be cached for offline use. If it is not
   available, graphs will not appear but text data and CSV downloads
   will still work.

   Copyright 2020-2023 Metriful Ltd.
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>
#include <WiFi_functions.h>
#include <graph_web_page.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// Choose how often to read and update data (every 3, 100, or 300 seconds)
// 100 or 300 seconds are recommended for long-term monitoring.
uint8_t cycle_period = CYCLE_PERIOD_3_S;

// The DATA_BUFFER_LENGTH parameter is the number of data points of each
// variable to store on the host. It is limited by the available host RAM.
#define DATA_BUFFER_LENGTH 576 
// Examples:
// For 16 hour graphs, choose 100 second cycle period and 576 buffer length
// For 24 hour graphs, choose 300 second cycle period and 288 buffer length

// Choose whether to create a new WiFi network (host as Access Point),
// or connect to an existing WiFi network.
bool createWifiNetwork = false;
// If creating a WiFi network, you must choose a static (fixed)
// IP address ("theIP").
// Otherwise, if connecting to an existing network, an IP address is
// automatically allocated. This program displays the IP address on
// the serial monitor, or you can find it on your router software.

// Provide the SSID (name) and password for the WiFi network. Depending
// on the choice of createWifiNetwork, this is either created by the 
// host (Access Point mode) or already exists.
// To avoid problems, do not create a network with the same SSID name
// as an already existing network.
// Also note: some boards (e.g. Pico and ESP8266) may not restart after
// being programmed in Access Point mode and require a power cycle.
const char * SSID = "PUT WIFI NETWORK NAME HERE"; // network SSID (name)
const char * password = "PUT WIFI PASSWORD HERE"; // must be at least 8 characters

// Choose a static IP address for the host, only used when generating 
// a new WiFi network (createWifiNetwork = true). The served web 
// page will be available at  http://<IP address here>
IPAddress theIP(192, 168, 12, 20); 
// e.g. theIP(192, 168, 12, 20) means an IP of 192.168.12.20
//      and the web page will be at http://192.168.12.20

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

#if !defined(HAS_WIFI)
#error ("This example program has been created for specific WiFi enabled hosts only.")
#endif

WiFiServer server(80);
uint16_t dataPeriod_s;

// Structs for data
AirData_F_t airDataF = {0};
AirQualityData_F_t airQualityDataF = {0};
LightData_F_t lightDataF = {0}; 
ParticleData_F_t particleDataF = {0};
SoundData_F_t soundDataF = {0};

uint16_t bufferLength = 0;
float temperature_buffer[DATA_BUFFER_LENGTH] = {0};
float pressure_buffer[DATA_BUFFER_LENGTH] = {0};
float humidity_buffer[DATA_BUFFER_LENGTH] = {0};
float AQI_buffer[DATA_BUFFER_LENGTH] = {0};
float bVOC_buffer[DATA_BUFFER_LENGTH] = {0};
float SPL_buffer[DATA_BUFFER_LENGTH] = {0};
float illuminance_buffer[DATA_BUFFER_LENGTH] = {0};
float particle_buffer[DATA_BUFFER_LENGTH] = {0};


void setup()
{
  // Initialize the host's pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 
  // Wait for serial to start functioning correctly:
  delay(2000);

  if (!wifiCreateOrConnect(createWifiNetwork, true, SSID, password, theIP))
  {
    Serial.println("Failed to set up WiFi.");
    while (true)
    {
      yield();
    }
  }

  // Start the web server
  server.begin();
  
  ////////////////////////////////////////////////////////////////////
  
  // Get time period value to send to web page
  if (cycle_period == CYCLE_PERIOD_3_S)
  {
    dataPeriod_s = 3;
  }
  else if (cycle_period == CYCLE_PERIOD_100_S)
  {
    dataPeriod_s = 100;
  }
  else
  { // CYCLE_PERIOD_300_S
    dataPeriod_s = 300;
  }
  
  // Apply the chosen settings to the Metriful board
  uint8_t particleSensor = PARTICLE_SENSOR;
  TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, &particleSensor, 1);
  TransmitI2C(I2C_ADDRESS, CYCLE_TIME_PERIOD_REG, &cycle_period, 1);
  ready_assertion_event = false;
  TransmitI2C(I2C_ADDRESS, CYCLE_MODE_CMD, 0, 0);
}

void loop()
{
  // Respond to the web page client requests while waiting for new data
  while (!ready_assertion_event)
  {
    handleClientRequests();
    yield();
  }
  ready_assertion_event = false;
  
  // Read the new data and convert to float types:
  airDataF = getAirDataF(I2C_ADDRESS);
  airQualityDataF = getAirQualityDataF(I2C_ADDRESS);
  lightDataF = getLightDataF(I2C_ADDRESS);
  soundDataF = getSoundDataF(I2C_ADDRESS);
  particleDataF = getParticleDataF(I2C_ADDRESS);
  
  // Save the data
  updateDataBuffers();
  
  // Check WiFi is still connected
  if (!createWifiNetwork)
  {
    uint8_t wifiStatus = WiFi.status();
    if (wifiStatus != WL_CONNECTED)
    {
      // There is a problem with the WiFi connection: attempt to reconnect.
      Serial.print("Wifi status: ");
      Serial.println(interpret_WiFi_status(wifiStatus));
      connectToWiFi(SSID, password);
      theIP = WiFi.localIP();
      Serial.print("Reconnected. View your page at http://");
      Serial.println(theIP);
      ready_assertion_event = false;
    }
  }
}

// Store the data, up to a maximum length of DATA_BUFFER_LENGTH, then start 
// discarding the oldest data in a FIFO scheme ("First In First Out")
void updateDataBuffers(void)
{
  uint16_t position = 0;
  if (bufferLength == DATA_BUFFER_LENGTH)
  {
    // Buffers are full: shift all data values along, discarding the oldest
    for (uint16_t i = 0; i < (DATA_BUFFER_LENGTH - 1); i++)
    {
      temperature_buffer[i] = temperature_buffer[i+1];
      pressure_buffer[i] = pressure_buffer[i+1];
      humidity_buffer[i] = humidity_buffer[i+1];
      AQI_buffer[i] = AQI_buffer[i+1];
      bVOC_buffer[i] = bVOC_buffer[i+1];
      SPL_buffer[i] = SPL_buffer[i+1];
      illuminance_buffer[i] = illuminance_buffer[i+1];
      particle_buffer[i] = particle_buffer[i+1];
    }
    position = DATA_BUFFER_LENGTH - 1;
  }
  else
  {
    // Buffers are not yet full; keep filling them
    position = bufferLength;
    bufferLength++;
  }

  // Save the new data in the buffers
  AQI_buffer[position] = airQualityDataF.AQI;
  #ifdef USE_FAHRENHEIT
    temperature_buffer[position] = convertCtoF(airDataF.T_C);
  #else
    temperature_buffer[position] = airDataF.T_C;
  #endif
  pressure_buffer[position] = (float) airDataF.P_Pa;
  humidity_buffer[position] = airDataF.H_pc;
  SPL_buffer[position] = soundDataF.SPL_dBA;
  illuminance_buffer[position] = lightDataF.illum_lux;
  bVOC_buffer[position] = airQualityDataF.bVOC;
  particle_buffer[position] = particleDataF.concentration;
}


#define GET_REQUEST_STR "GET /"
#define URI_CHARS 2
// Send either the web page or the data in response to HTTP requests.
void handleClientRequests(void)
{
  // Check for incoming client requests
  WiFiClient client = getClient(&server);
  if (client)
  {
    uint8_t requestCount = 0;
    char requestBuffer[sizeof(GET_REQUEST_STR)] = {0};
    uint8_t uriCount = 0;
    char uriBuffer[URI_CHARS] = {0};

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();

        if (requestCount < (sizeof(GET_REQUEST_STR)-1))
        {
          // Assemble the first part of the message containing the
          // HTTP method (GET, POST etc)
          requestBuffer[requestCount] = c;
          requestCount++;
        }
        else if (uriCount < URI_CHARS)
        {
          // Assemble the URI, up to a fixed number of characters
          uriBuffer[uriCount] = c;
          uriCount++;
        }
        else
        {
          // Now use the assembled method and URI to decide how to respond
          if (strcmp(requestBuffer, GET_REQUEST_STR) == 0)
          {
            // It is a GET request (no other methods are supported).
            // Now check for valid URIs.
            if (uriBuffer[0] == ' ')
            {
              // The web page is requested
              client.write(pageHeader, strlen(pageHeader));
              sendData(&client, (const uint8_t *) graphWebPage, strlen(graphWebPage));
              break; 
            }
            else if ((uriBuffer[0] == '1') && (uriBuffer[1] == ' '))
            {
              // A URI of '1' indicates a request of all buffered data
              sendAllData(&client);
              break; 
            }
            else if ((uriBuffer[0] == '2') && (uriBuffer[1] == ' '))
            {
              // A URI of '2' indicates a request of the latest data only
              sendLatestData(&client);
              break;
            }
          }
          // Reaching here means that the request is not supported or
          // is incorrect (not a GET request, or not a valid URI)
          // so send an error.
          client.print(errorResponseHTTP);
          break;
        }
      }
    }
    #if !defined(ESP8266) && !defined(ARDUINO_ARCH_RP2040)
      client.stop();
    #endif
  }
}

// Send all buffered data in the HTTP response. Binary format ("octet-stream")
// is used, and the receiving web page uses the known order of the data to 
// decode and interpret it.
void sendAllData(WiFiClient * clientPtr)
{
  clientPtr->print(dataHeader);
  // First send the time period
  clientPtr->write((const uint8_t *) &dataPeriod_s, sizeof(uint16_t));
  // Send particle sensor type
  uint8_t codeByte = (uint8_t) PARTICLE_SENSOR;
  clientPtr->write((const uint8_t *) &codeByte, sizeof(uint8_t));
  // Send temperature unit
  codeByte = 0;
  #ifdef USE_FAHRENHEIT
    codeByte = 1;
  #endif
  clientPtr->write((const uint8_t *) &codeByte, sizeof(uint8_t));
  // Send the length of the data buffers (number of values of each variable)
  clientPtr->write((const uint8_t *) &bufferLength, sizeof(uint16_t));
  // Send the data, unless none have been read yet:
  if (bufferLength > 0)
  {
    sendData(clientPtr, (const uint8_t *) AQI_buffer, bufferLength*sizeof(float));
    sendData(clientPtr, (const uint8_t *) temperature_buffer, bufferLength*sizeof(float));
    sendData(clientPtr, (const uint8_t *) pressure_buffer, bufferLength*sizeof(float));
    sendData(clientPtr, (const uint8_t *) humidity_buffer, bufferLength*sizeof(float));
    sendData(clientPtr, (const uint8_t *) SPL_buffer, bufferLength*sizeof(float));
    sendData(clientPtr, (const uint8_t *) illuminance_buffer, bufferLength*sizeof(float));
    sendData(clientPtr, (const uint8_t *) bVOC_buffer, bufferLength*sizeof(float));
    if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF) {
      sendData(clientPtr, (const uint8_t *) particle_buffer, bufferLength*sizeof(float));
    }
  }
}


// Send just the most recent value of each variable (or no data if no values
// have been read yet)
void sendLatestData(WiFiClient * clientPtr)
{
  clientPtr->print(dataHeader);
  if (bufferLength > 0)
  {
    uint16_t bufferPosition = bufferLength - 1;
    clientPtr->write((const uint8_t *) &(AQI_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(temperature_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(pressure_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(humidity_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(SPL_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(illuminance_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(bVOC_buffer[bufferPosition]), sizeof(float));
    if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF)
    {
      clientPtr->write((const uint8_t *) &(particle_buffer[bufferPosition]), sizeof(float));
    }
  }
}


// client.write() may fail with very large inputs, so split
// into several separate write() calls with a short delay between each.
#define MAX_DATA_BYTES 1000
void sendData(WiFiClient * clientPtr, const uint8_t * dataPtr, size_t dataLength)
{
  while (dataLength > 0)
  {
    size_t sendLength = dataLength;
    if (sendLength > MAX_DATA_BYTES)
    {
      sendLength = MAX_DATA_BYTES;
    }
    clientPtr->write(dataPtr, sendLength);
    delay(10);
    dataLength -= sendLength;
    dataPtr += sendLength;
  }
}
