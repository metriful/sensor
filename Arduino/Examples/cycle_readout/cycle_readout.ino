/* 
   cycle_readout.ino

   Example code for using the Metriful MS430 in cycle mode. 
   
   Continually measures and displays all environment data in 
   a repeating cycle. User can choose from a cycle time period 
   of 3, 100, or 300 seconds. View the output in the Serial Monitor.

   The measurements can be displayed as either labeled text, or as 
   simple columns of numbers.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// How often to read data (every 3, 100, or 300 seconds)
uint8_t cycle_period = CYCLE_PERIOD_3_S;

// How to print the data over the serial port. If printDataAsColumns = true,
// data are columns of numbers, useful to copy/paste to a spreadsheet
// application. Otherwise, data are printed with explanatory labels and units.
bool printDataAsColumns = false;

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

// Structs for data
AirData_t airData = {0};
AirQualityData_t airQualityData = {0};
LightData_t lightData = {0};
SoundData_t soundData = {0};
ParticleData_t particleData = {0};


void setup() {  
  // Initialize the host pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 
  
  // Apply chosen settings to the MS430
  uint8_t particleSensor = PARTICLE_SENSOR;
  TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, &particleSensor, 1);
  TransmitI2C(I2C_ADDRESS, CYCLE_TIME_PERIOD_REG, &cycle_period, 1);

  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {
    yield();
  } 

  Serial.println("Entering cycle mode and waiting for data.");
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
  
  // Air data
  // Choose output temperature unit (C or F) in Metriful_sensor.h
  airData = getAirData(I2C_ADDRESS);
  
  /* Air quality data
  The initial self-calibration of the air quality data may take several
  minutes to complete. During this time the accuracy parameter is zero 
  and the data values are not valid.
  */ 
  airQualityData = getAirQualityData(I2C_ADDRESS);
  
  // Light data
  lightData = getLightData(I2C_ADDRESS);
  
  // Sound data
  soundData = getSoundData(I2C_ADDRESS);
  
  /* Particle data
  This requires the connection of a particulate sensor (invalid 
  values will be obtained if this sensor is not present).
  Specify your sensor model (PPD42 or SDS011) in Metriful_sensor.h
  Also note that, due to the low pass filtering used, the 
  particle data become valid after an initial initialization 
  period of approximately one minute.
  */ 
  if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF) {
    particleData = getParticleData(I2C_ADDRESS);
  }

  // Print all data to the serial port
  printAirData(&airData, printDataAsColumns);
  printAirQualityData(&airQualityData, printDataAsColumns);
  printLightData(&lightData, printDataAsColumns);
  printSoundData(&soundData, printDataAsColumns);
  if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF) {
    printParticleData(&particleData, printDataAsColumns, PARTICLE_SENSOR);
  }
  Serial.println();
}
