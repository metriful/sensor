/* 
   simple_read_sound.ino

   Example code for using the Sense board to measure sound. 
   
   Waits for microphone initialization, then measures and displays 
   the sound data. View the output in the Serial Monitor.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit https://github.com/metriful/sense
*/

#include <Metriful_Sense.h>
#include <stdint.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// The I2C address of the Sense board
uint8_t i2c_7bit_address = I2C_ADDR_7BIT_SB_OPEN;

// Whether to use floating point representation of numbers in the 
// Arduino (uses more resources)
bool useFloatingPoint = false;

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

// Ensure tx/rx buffers are big enough to fit the largest send/receive transactions:
uint8_t transmit_buffer[LIGHT_INTERRUPT_THRESHOLD_BYTES] = {0};
uint8_t receive_buffer[SOUND_DATA_BYTES] = {0};

void setup() {  
  // Initialize the Arduino pins, set up the serial port and reset:
  SenseHardwareSetup(i2c_7bit_address); 
  
  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {} 
  
  ////////////////////////////////////////////////////////////////////
  
  // Wait for the microphone to stabilize (takes approximately 1.5 seconds). 
  // The microphone uses a filter which should be allowed to settle before 
  // sound data are measured. 
  // This only needs to be done once after Sense is powered-on or reset.
  bool mic_stable = false;
  while (!mic_stable) {
    // The sound_stable register is set to a non-zero value when the microphone has initialized
    ReceiveI2C(i2c_7bit_address, SOUND_STABLE_READ, receive_buffer, SOUND_STABLE_BYTES);
    mic_stable = (receive_buffer[0] != 0);
    delay(0.05);
  }
  
  ////////////////////////////////////////////////////////////////////

  // Clear the global variable in preparation for waiting for READY assertion
  ready_assertion_event = false;
  
  // Initiate an on-demand data measurement
  TransmitI2C(i2c_7bit_address, ON_DEMAND_MEASURE_CMD, transmit_buffer, 0);

  // Now wait for the ready signal (falling edge) before continuing
  while (!ready_assertion_event) {}
    
  // We now know that newly measured data are ready to read.

  ////////////////////////////////////////////////////////////////////

  // SOUND DATA

  // Read all sound data in one transaction, interpreting the received bytes as a SoundData_t struct:
  SoundData_t soundData = {0};
  ReceiveI2C(i2c_7bit_address, SOUND_DATA_READ, (uint8_t *) &soundData, SOUND_DATA_BYTES);
  
  // Print the values over the serial port
  if (useFloatingPoint) {
    // Convert values where necessary to floating point representation:
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
