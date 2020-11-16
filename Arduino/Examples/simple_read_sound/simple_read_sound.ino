/* 
   simple_read_sound.ino

   Example code for using the Metriful MS430 to measure sound. 
   
   Demonstrates multiple ways of reading and displaying the sound data. 
   View the output in the Serial Monitor. The other data can be measured
   and displayed in a similar way.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>


void setup() {  
  // Initialize the host pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 
  
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
  TransmitI2C(I2C_ADDRESS, ON_DEMAND_MEASURE_CMD, 0, 0);

  // Now wait for the ready signal (falling edge) before continuing
  while (!ready_assertion_event) {
    yield();
  }
    
  // We now know that newly measured data are ready to read.

  ////////////////////////////////////////////////////////////////////
  
  // There are multiple ways to read and display the data
  
  
  // 1. Simplest way: use the example float (_F) functions
  
  // Read the sound data from the board
  SoundData_F_t soundDataF = getSoundDataF(I2C_ADDRESS);
  
  // Print all of the sound measurements to the serial monitor
  printSoundDataF(&soundDataF);
  
  Serial.println("-----------------------------");
  
  
  // 2. After reading from the MS430, you can also access and print the 
  // float data directly from the struct:
  Serial.print("The sound pressure level is: ");
  Serial.print(soundDataF.SPL_dBA, 1);   // print to 1 decimal place
  Serial.println(" dBA");
  
  Serial.println("-----------------------------");
  
  
  // 3. If host resources are limited, avoid using floating point and 
  // instead use the integer versions (without "F" in the name)
  SoundData_t soundData = getSoundData(I2C_ADDRESS);
  
  // Print to the serial monitor
  printSoundData(&soundData, false); 
  // If the second argument is "true", data are printed as columns.
  
  Serial.println("-----------------------------");
  
  
  // 4. Access and print integer data directly from the struct:
  Serial.print("The sound pressure level is: ");
  Serial.print(soundData.SPL_dBA_int);    // the integer part of the value
  Serial.print(".");                      // the decimal point
  Serial.print(soundData.SPL_dBA_fr_1dp); // the fractional part (1 decimal place) 
  Serial.println(" dBA");

  Serial.println("-----------------------------");
}

void loop() {
  // There is no loop for this program.
}
