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
   
   The host can either connect to an existing WiFi network, or generate 
   its own for other devices to connect to (Access Point mode).
    
   The browser which views the web page uses the Plotly javascript 
   library to generate the graphs. This is automatically downloaded 
   over the internet, or can be cached for offline use. If it is not 
   available, graphs will not appear but text data and CSV downloads 
   should still work.

   Copyright 2020 Metriful Ltd. 
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
uint8_t cycle_period = CYCLE_PERIOD_100_S;

// The BUFFER_LENGTH parameter is the number of data points of each
// variable to store on the host. It is limited by the available host RAM.
#define BUFFER_LENGTH 576 
// Examples:
// For 16 hour graphs, choose 100 second cycle period and 576 buffer length
// For 24 hour graphs, choose 300 second cycle period and 288 buffer length

// Choose whether to create a new WiFi network (host as Access Point),
// or connect to an existing WiFi network.
bool createWifiNetwork = true;
// If creating a WiFi network, a static (fixed) IP address ("theIP") is 
// specified by the user.  Otherwise, if connecting to an existing 
// network, an IP address is automatically allocated and the serial 
// output must be viewed at startup to see this allocated IP address.

// Provide the SSID (name) and password for the WiFi network. Depending
// on the choice of createWifiNetwork, this is either created by the 
// host (Access Point mode) or already exists.
// To avoid problems, do not create a network with the same SSID name
// as an already existing network.
char SSID[] = "PUT WIFI NETWORK NAME HERE IN QUOTES"; // network SSID (name)
char password[] = "PUT WIFI PASSWORD HERE IN QUOTES"; // network password; must be at least 8 characters

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

const char * errorResponseHTTP = "HTTP/1.1 400 Bad Request\r\n\r\n";

const char * dataHeader = "HTTP/1.1 200 OK\r\n"
                          "Content-type: application/octet-stream\r\n" 
                          "Connection: close\r\n\r\n";

uint16_t bufferLength = 0;
float temperature_buffer[BUFFER_LENGTH] = {0};
float pressure_buffer[BUFFER_LENGTH] = {0};
float humidity_buffer[BUFFER_LENGTH] = {0};
float AQI_buffer[BUFFER_LENGTH] = {0};
float bVOC_buffer[BUFFER_LENGTH] = {0};
float SPL_buffer[BUFFER_LENGTH] = {0};
float illuminance_buffer[BUFFER_LENGTH] = {0};
float particle_buffer[BUFFER_LENGTH] = {0};


void setup() {
  // Initialize the host's pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 
  
  if (createWifiNetwork) {
    // The host generates its own WiFi network ("Access Point") with
    // a chosen static IP address
    if (!createWiFiAP(SSID, password, theIP)) {
      Serial.println("Failed to create access point.");
      while (true) {
        yield();
      }
    }
  }
  else {
    // The host connects to an existing Wifi network
    
    // Wait for the serial port to start because the user must be able
    // to see the printed IP address in the serial monitor
    while (!Serial) {
      yield();
    }
    
    // Attempt to connect to the Wifi network and obtain the IP
    // address. Because the address is not known before this point,
    // a serial monitor must be used to display it to the user.
    connectToWiFi(SSID, password);
    theIP = WiFi.localIP();
  }
 
  // Print the IP address: use this address in a browser to view the 
  // generated web page
  Serial.print("View your page at http://");
  Serial.println(theIP);

  // Start the web server
  server.begin();
  
  ////////////////////////////////////////////////////////////////////
  
  // Get time period value to send to web page
  if (cycle_period == CYCLE_PERIOD_3_S) {
    dataPeriod_s = 3;
  }
  else if (cycle_period == CYCLE_PERIOD_100_S) {
    dataPeriod_s = 100;
  }
  else { // CYCLE_PERIOD_300_S
    dataPeriod_s = 300;
  }
  
  // Apply the chosen settings to the Metriful board
  uint8_t particleSensor = PARTICLE_SENSOR;
  TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, &particleSensor, 1);
  TransmitI2C(I2C_ADDRESS, CYCLE_TIME_PERIOD_REG, &cycle_period, 1);
  ready_assertion_event = false;
  TransmitI2C(I2C_ADDRESS, CYCLE_MODE_CMD, 0, 0);
}

void loop() {

  // Respond to the web page client requests while waiting for new data
  while (!ready_assertion_event) {
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
  if (!createWifiNetwork) {
    uint8_t wifiStatus = WiFi.status();
    if (wifiStatus != WL_CONNECTED) {
      // There is a problem with the WiFi connection: attempt to reconnect.
      Serial.print("Wifi status: ");
      Serial.println(interpret_WiFi_status(wifiStatus));
      connectToWiFi(SSID, password);
      theIP = WiFi.localIP();
      Serial.print("View your page at http://");
      Serial.println(theIP);
      ready_assertion_event = false;
    }
  }
}

// Store the data, up to a maximum length of BUFFER_LENGTH, then start 
// discarding the oldest data in a FIFO scheme ("First In First Out")
void updateDataBuffers(void) {
  uint16_t position = 0;
  if (bufferLength == BUFFER_LENGTH) {
    // Buffers are full: shift all data values along, discarding the oldest
    for (uint16_t i=0; i<(BUFFER_LENGTH-1); i++) {
      temperature_buffer[i] = temperature_buffer[i+1];
      pressure_buffer[i] = pressure_buffer[i+1];
      humidity_buffer[i] = humidity_buffer[i+1];
      AQI_buffer[i] = AQI_buffer[i+1];
      bVOC_buffer[i] = bVOC_buffer[i+1];
      SPL_buffer[i] = SPL_buffer[i+1];
      illuminance_buffer[i] = illuminance_buffer[i+1];
      particle_buffer[i] = particle_buffer[i+1];
    }
    position = BUFFER_LENGTH-1;
  }
  else {
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
void handleClientRequests(void) {
  // Check for incoming client requests
  WiFiClient client = server.available();
  if (client) {

    uint8_t requestCount = 0;
    char requestBuffer[sizeof(GET_REQUEST_STR)] = {0};

    uint8_t uriCount = 0;
    char uriBuffer[URI_CHARS] = {0};

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (requestCount < (sizeof(GET_REQUEST_STR)-1)) {
          // Assemble the first part of the message containing the HTTP method (GET, POST etc)
          requestBuffer[requestCount] = c;
          requestCount++;
        }
        else if (uriCount < URI_CHARS) {
          // Assemble the URI, up to a fixed number of characters
          uriBuffer[uriCount] = c;
          uriCount++;
        }
        else {
          // Now use the assembled method and URI to decide how to respond
          if (strcmp(requestBuffer, GET_REQUEST_STR) == 0) {
            // It is a GET request (no other methods are supported).
            // Now check for valid URIs.
            if (uriBuffer[0] == ' ') {
              // The web page is requested
              sendData(&client, (const uint8_t *) graphWebPage, strlen(graphWebPage));
              break; 
            }
            else if ((uriBuffer[0] == '1') && (uriBuffer[1] == ' ')) {
              // A URI of '1' indicates a request of all buffered data
              sendAllData(&client);
              break; 
            }
            else if ((uriBuffer[0] == '2') && (uriBuffer[1] == ' ')) {
              // A URI of '2' indicates a request of the latest data only
              sendLatestData(&client);
              break;
            }
          }
          // Reaching here means that the request is not supported or is incorrect
          // (not a GET request, or not a valid URI) so send an error.
          client.print(errorResponseHTTP);
          break;
        }
      }
    }
    #ifndef ESP8266
      client.stop();
    #endif
  }
}

// Send all buffered data in the HTTP response. Binary format ("octet-stream") 
// is used, and the receiving web page uses the known order of the data to 
// decode and interpret it.
void sendAllData(WiFiClient * clientPtr) {
  clientPtr->print(dataHeader);
  // First send the time period, so the web page knows when to do the next request
  clientPtr->write((const uint8_t *) &dataPeriod_s, sizeof(uint16_t));
  // Send temperature unit and particle sensor type, combined into one byte
  uint8_t codeByte = (uint8_t) PARTICLE_SENSOR;
  #ifdef USE_FAHRENHEIT
    codeByte = codeByte | 0x10;
  #endif
  clientPtr->write((const uint8_t *) &codeByte, sizeof(uint8_t));
  // Send the length of the data buffers (the number of values of each variable)
  clientPtr->write((const uint8_t *) &bufferLength, sizeof(uint16_t));
  // Send the data, unless none have been read yet:
  if (bufferLength > 0) {
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
void sendLatestData(WiFiClient * clientPtr) {
  clientPtr->print(dataHeader);
  if (bufferLength > 0) {
    uint16_t bufferPosition = bufferLength-1;
    clientPtr->write((const uint8_t *) &(AQI_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(temperature_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(pressure_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(humidity_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(SPL_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(illuminance_buffer[bufferPosition]), sizeof(float));
    clientPtr->write((const uint8_t *) &(bVOC_buffer[bufferPosition]), sizeof(float));
    if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF) {
      clientPtr->write((const uint8_t *) &(particle_buffer[bufferPosition]), sizeof(float));
    }
  }
}


// client.write() may fail with very large inputs, so split
// into several separate write() calls with a short delay between each.
#define MAX_DATA_BYTES 1000
void sendData(WiFiClient * clientPtr, const uint8_t * dataPtr, size_t dataLength) {
  while (dataLength > 0) {
    size_t sendLength = dataLength;
    if (sendLength > MAX_DATA_BYTES) {
      sendLength = MAX_DATA_BYTES;
    }
    clientPtr->write(dataPtr, sendLength);
    delay(10);
    dataLength-=sendLength;
    dataPtr+=sendLength;
  }
}
