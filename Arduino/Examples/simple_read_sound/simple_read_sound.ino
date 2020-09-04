/* 
   simple_read_sound.ino

   Example code for using the Metriful MS430 to measure sound. 
   
   Measures and displays the sound data once. 
   View the output in the Serial Monitor.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// The I2C address of the Metriful board
uint8_t i2c_7bit_address = I2C_ADDR_7BIT_SB_OPEN;

// Whether to use floating point representation of numbers (uses more 
// host resources)
bool useFloatingPoint = false;

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

uint8_t receive_buffer[1] = {0};

void setup() {  
  // Initialize the host pins, set up the serial port and reset:
  SensorHardwareSetup(i2c_7bit_address); 
  
  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {
    yield();
  } 
  
  ////////////////////////////////////////////////////////////////////
  
  // Wait for the microphone signal to stabilize (takes approximately 1.5 seconds). 
  // This only needs to be done once after the MS430 is powered-on or reset.
  delay(1500);
  
  ////////////////////////////////////////////////////////////////////

  // Clear the global variable in preparation for waiting for READY assertion
  ready_assertion_event = false;
  
  // Initiate an on-demand data measurement
  TransmitI2C(i2c_7bit_address, ON_DEMAND_MEASURE_CMD, 0, 0);

  // Now wait for the ready signal (falling edge) before continuing
  while (!ready_assertion_event) {
    yield();
  }
    
  // We now know that newly measured data are ready to read.

  ////////////////////////////////////////////////////////////////////

  // Read all sound data in one transaction, interpreting the received bytes as a SoundData_t struct:
  SoundData_t soundData = {0};
  ReceiveI2C(i2c_7bit_address, SOUND_DATA_READ, (uint8_t *) &soundData, SOUND_DATA_BYTES);
  
  // Print the values over the serial port
  if (useFloatingPoint) {
    // Convert values to floating point representation:
    SoundData_F_t soundDataF = {0};
    convertSoundDataF(&soundData, &soundDataF);
    printSoundDataF(&soundDataF);
  }
  else {
    // The data remain in integer representation 
    printSoundData(&soundData, false);
  }
}

void loop() {
  // There is no loop for this program.
}
