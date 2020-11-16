/* 
   on_demand_readout.ino

   Example code for using the Metriful MS430 in "on-demand" mode. 
   
   Repeatedly measures and displays all environment data, with a pause
   between measurements. Air quality data are unavailable in this mode 
   (instead see cycle_readout.ino). View output in the Serial Monitor.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// Pause (in milliseconds) between data measurements (note that the
// measurement itself takes 0.5 seconds)
uint32_t pause_ms = 4500;
// Choosing a pause of less than 2000 ms will cause inaccurate 
// temperature, humidity and particle data.

// How to print the data over the serial port. If printDataAsColumns = true,
// data are columns of numbers, useful to copy/paste to a spreadsheet
// application. Otherwise, data are printed with explanatory labels and units.
bool printDataAsColumns = false;

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

// Structs for data
AirData_t airData = {0};
LightData_t lightData = {0};
SoundData_t soundData = {0};
ParticleData_t particleData = {0};


void setup() {  
  // Initialize the host pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 
  
  uint8_t particleSensor = PARTICLE_SENSOR;
  TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, &particleSensor, 1);

  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {
    yield();
  } 
}


void loop() {

  // Trigger a new measurement
  ready_assertion_event = false;
  TransmitI2C(I2C_ADDRESS, ON_DEMAND_MEASURE_CMD, 0, 0);

  // Wait for the measurement to finish, indicated by a falling edge on READY
  while (!ready_assertion_event) {
    yield();
  }
  
  // Read data from the MS430 into the data structs. 
  
  // Air data
  // Choose output temperature unit (C or F) in Metriful_sensor.h
  airData = getAirData(I2C_ADDRESS);
  
  // Air quality data are not available with on demand measurements
  
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
  printLightData(&lightData, printDataAsColumns);
  printSoundData(&soundData, printDataAsColumns);
  if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF) {
    printParticleData(&particleData, printDataAsColumns, PARTICLE_SENSOR);
  }
  Serial.println();

  // Wait for the chosen time period before repeating everything
  delay(pause_ms);
}
