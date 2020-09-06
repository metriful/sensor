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
uint32_t pause_ms = 3500;
// Choosing a pause of less than 2000 ms will cause inaccurate 
// temperature, humidity and particle data.

// The I2C address of the Metriful board
uint8_t i2c_7bit_address = I2C_ADDR_7BIT_SB_OPEN;

// Which particle sensor is attached (PPD42, SDS011, or OFF)
ParticleSensor_t particleSensor = OFF;

// How to print the data over the serial port. If printDataAsColumns = true,
// data are columns of numbers, useful to copy/paste to a spreadsheet
// application. Otherwise, data are printed with explanatory labels and units.
bool printDataAsColumns = true;

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

uint8_t transmit_buffer[1] = {0};

// Structs for data
AirData_t airData = {0};
LightData_t lightData = {0};
SoundData_t soundData = {0};
ParticleData_t particleData = {0};


void setup() {  
  // Initialize the host pins, set up the serial port and reset:
  SensorHardwareSetup(i2c_7bit_address); 
  
  if (particleSensor != OFF) {
    transmit_buffer[0] = particleSensor;
    TransmitI2C(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, transmit_buffer, 1);
  }

  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {
    yield();
  } 
}


void loop() {

  // Trigger a new measurement
  ready_assertion_event = false;
  TransmitI2C(i2c_7bit_address, ON_DEMAND_MEASURE_CMD, 0, 0);

  // Wait for the measurement to finish, indicated by a falling edge on READY
  while (!ready_assertion_event) {
    yield();
  }

  /* Read data from the MS430 into the data structs. 
  For each category of data (air, sound, etc.) a pointer to the data struct is 
  passed to the ReceiveI2C() function. The received byte sequence fills the data 
  struct in the correct order so that each field within the struct receives
  the value of an environmental quantity (temperature, sound level, etc.)
  */ 
  
  // Air data
  ReceiveI2C(i2c_7bit_address, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);
  
  // Air quality data are not available with on demand measurements
  
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

  // Print all data to the serial port
  printAirData(&airData, printDataAsColumns);
  printLightData(&lightData, printDataAsColumns);
  printSoundData(&soundData, printDataAsColumns);
  if (particleSensor != OFF) {
    printParticleData(&particleData, printDataAsColumns, particleSensor);
  }
  Serial.println();

  delay(pause_ms);
}
