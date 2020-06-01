/* 
   Arduino_pin_definitions.h

   This file defines which Arduino pins are used to interface to the 
   Sense board and which library is used for I2C communications.

   The relevant section in this file is selected automatically when 
   the Arduino board is chosen in the Arduino IDE. The Arduino 
   Nano 33 IoT requires the use of a software I2C library; the examples 
   use the SlowSoftWire library but alternatives are available.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit https://github.com/metriful/sense
*/

#ifndef ARDUINO_PIN_DEFINITIONS_H
#define ARDUINO_PIN_DEFINITIONS_H

#ifdef ARDUINO_AVR_UNO

  // Arduino Uno

  #include <Wire.h>
  #define TheWire Wire

  // I2C pins are set by default so not defined here
  #define READY_PIN 2   // Arduino digital pin 2 connects to RDY
  #define L_INT_PIN 4   // Arduino digital pin 4 connects to LIT
  #define S_INT_PIN 7   // Arduino digital pin 7 connects to SIT
  /* In addition to these pins, the following I2C bus and power 
   connections must be made to the Sense board when
   using the Arduino Uno (a 5V system): 
   Arduino pins GND, SCL, SDA to Sense board pins GND, SCL, SDA
   Arduino pin 5V to VPU and VIN
   Sense pin VDD is unused
   
   If a PPD42 particle sensor is used, also connect the following:
   Arduino pin 5V to PPD42 pin 3
   Arduino pin GND to PPD42 pin 1
   PPD42 pin 4 to Sense pin PRT
   
   For further details, see the readme and User Guide
  */

#elif defined(ARDUINO_SAMD_NANO_33_IOT)

  // Arduino Nano 33 IoT
  
  // The Wifi module uses the built-in hardware-supported I2C pins,
  // so a software I2C library must be used with pins SOFT_SDA and SOFT_SCL.
  // The examples use the SlowSoftWire library but alternatives are available.
  #include <SlowSoftWire.h>

  #define READY_PIN 13  // Arduino pin D13 connects to RDY
  #define L_INT_PIN A1  // Arduino pin A1 connects to LIT
  #define S_INT_PIN A2  // Arduino pin A2 connects to SIT
  #define SOFT_SDA A0   // Arduino pin A0 connects to SDA
  #define SOFT_SCL A3   // Arduino pin A3 connects to SCL
  /* In addition to these pins, the following I2C bus and power 
   connections must be made to the Sense board when
   using the Arduino Nano 33 IoT (a 3.3V system): 
   Arduino pin GND to Sense board GND
   Arduino pin 3.3V to VPU and VDD
   Sense pin VIN is unused
   
   If a PPD42 particle sensor is used, also connect the following:
   Arduino pin VUSB to PPD42 pin 3
   Arduino pin GND to PPD42 pin 1
   PPD42 pin 4 to Sense pin PRT
   The solder bridge labeled "VUSB" on the underside of the Arduino 
   must be soldered closed to provide 5V to the PPD42.
   
   For further details, see the readme and User Guide
  */

#else
  #error ("Boards other than Arduino Uno and Nano 33 IoT must have their pins defined by the user.")
#endif

#endif
