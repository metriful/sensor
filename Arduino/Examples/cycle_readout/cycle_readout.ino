/* 
   cycle_readout.ino

   Example code for using the Metriful MS430 in cycle mode. 
   
   Continually measures and displays all environment data in 
   a repeating cycle. User can choose from a cycle time period 
   of 3, 100, or 300 seconds. View the output in the Serial Monitor.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// How often to read data (every 3, 100, 300 seconds)
uint8_t cycle_period = CYCLE_PERIOD_3_S;

// The I2C address of the Metriful board
uint8_t i2c_7bit_address = I2C_ADDR_7BIT_SB_OPEN;

// Which particle sensor is attached (PPD42, SDS011, or OFF)
ParticleSensor_t particleSensor = OFF;

// How to print the data over the serial port. If printDataAsColumns = true,
// data are columns of numbers, useful to copy/paste to a spreadsheet
// application. Otherwise, data are printed with explanatory labels and units.
bool printDataAsColumns = false;

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

uint8_t transmit_buffer[1] = {0};

// Structs for data
AirData_t airData = {0};
AirQualityData_t airQualityData = {0};
LightData_t lightData = {0};
SoundData_t soundData = {0};
ParticleData_t particleData = {0};


void setup() {  
  // Initialize the host pins, set up the serial port and reset:
  SensorHardwareSetup(i2c_7bit_address); 
  
  // Apply chosen settings to the MS430
  if (particleSensor != OFF) {
    transmit_buffer[0] = particleSensor;
    TransmitI2C(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, transmit_buffer, 1);
  }
  transmit_buffer[0] = cycle_period;
  TransmitI2C(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, transmit_buffer, 1);

  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {
    yield();
  } 

  Serial.println("Entering cycle mode and waiting for data.");
  ready_assertion_event = false;
  TransmitI2C(i2c_7bit_address, CYCLE_MODE_CMD, 0, 0);
}


void loop() {
  // Wait for the next new data release, indicated by a falling edge on READY
  while (!ready_assertion_event) {
    yield();
  }
  ready_assertion_event = false;

  /* Read data from the MS430 into the data structs. 
  For each category of data (air, sound, etc.) a pointer to the data struct is 
  passed to the ReceiveI2C() function. The received byte sequence fills the data 
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

  // Print all data to the serial port
  printAirData(&airData, printDataAsColumns);
  printAirQualityData(&airQualityData, printDataAsColumns);
  printLightData(&lightData, printDataAsColumns);
  printSoundData(&soundData, printDataAsColumns);
  if (particleSensor != OFF) {
    printParticleData(&particleData, printDataAsColumns, particleSensor);
  }
  Serial.println();
}
