/* 
   host_pin_definitions.h

   This file defines which host pins are used to interface to the 
   Metriful MS430 board and which library is used for I2C communications.
    
   The relevant file section is selected automatically when the board 
   is chosen in the Arduino IDE.

   This file can be used with the following host systems:
   * Arduino Uno
   * Arduino Nano 33 IoT
   * Arduino Nano
   * Arduino MKR WiFi 1010
   * NodeMCU ESP8266
   The Metriful MS430 is compatible with many more development boards
   than just these five. You can use this file as a guide to define the
   necessary settings for other host systems.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
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
  connections must be made to the Metriful MS430:
  Arduino pins GND, SCL, SDA to MS430 pins GND, SCL, SDA
  Arduino pin 5V to MS430 pins VPU and VIN
  MS430 pin VDD is unused
   
  If a PPD42 particle sensor is used, connect the following:
  Arduino pin 5V to PPD42 pin 3
  Arduino pin GND to PPD42 pin 1
  PPD42 pin 4 to MS430 pin PRT
  
  If an SDS011 particle sensor is used, connect the following:
  Arduino pin 5V to SDS011 pin "5V"
  Arduino pin GND to SDS011 pin "GND"
  SDS011 pin "25um" to MS430 pin PRT
   
  For further details, see the readme and User Guide
  */

#elif defined ARDUINO_SAMD_NANO_33_IOT

  // Arduino Nano 33 IoT
  
  #include <SPI.h>
  #include <WiFiNINA.h>
  
  // The Wifi module uses the built-in hardware-supported I2C pins,
  // so a software I2C library must be used with pins SOFT_SDA and 
  // SOFT_SCL. The examples use the SlowSoftWire library but 
  // alternatives are available.
  #include <SlowSoftWire.h>

  #define READY_PIN 11  // Arduino pin D11 connects to RDY
  #define L_INT_PIN A1  // Arduino pin A1 connects to LIT
  #define S_INT_PIN A2  // Arduino pin A2 connects to SIT
  #define SOFT_SDA A0   // Arduino pin A0 connects to SDA
  #define SOFT_SCL A3   // Arduino pin A3 connects to SCL
  /* In addition to these pins, the following I2C bus and power 
  connections must be made to the Metriful MS430:
  Arduino pin GND to MS430 pin GND
  Arduino pin 3.3V to MS430 pins VPU and VDD
  MS430 pin VIN is unused
   
  If a PPD42 particle sensor is used, connect the following:
  Arduino pin VUSB to PPD42 pin 3
  Arduino pin GND to PPD42 pin 1
  PPD42 pin 4 to MS430 pin PRT
   
  If an SDS011 particle sensor is used, connect the following:
  Arduino pin VUSB to SDS011 pin "5V"
  Arduino pin GND to SDS011 pin "GND"
  SDS011 pin "25um" to MS430 pin PRT
   
  The solder bridge labeled "VUSB" on the underside of the Arduino 
  must be soldered closed to provide 5V to the PPD42/SDS011.
  
  For further details, see the readme and User Guide
  */
  
#elif defined ARDUINO_AVR_NANO

  // Arduino Nano

  #include <Wire.h>
  #define TheWire Wire

  // I2C pins (SDA = A4, SCL = A5) are set by default so not defined here
  #define READY_PIN 2   // Arduino pin D2 connects to RDY
  #define L_INT_PIN 4   // Arduino pin D4 connects to LIT
  #define S_INT_PIN 7   // Arduino pin D7 connects to SIT
  /* In addition to these pins, the following I2C bus and power 
  connections must be made to the Metriful MS430: 
  Arduino pins GND, A5 (SCL), A4 (SDA) to MS430 pins GND, SCL, SDA
  Arduino pin 5V to MS430 pins VPU and VIN
  MS430 pin VDD is unused
  
  If a PPD42 particle sensor is used, connect the following:
  Arduino pin 5V to PPD42 pin 3
  Arduino pin GND to PPD42 pin 1
  PPD42 pin 4 to MS430 pin PRT
   
  If an SDS011 particle sensor is used, connect the following:
  Arduino pin 5V to SDS011 pin "5V"
  Arduino pin GND to SDS011 pin "GND"
  SDS011 pin "25um" to MS430 pin PRT
   
  For further details, see the readme and User Guide
  */
  
#elif defined ARDUINO_SAMD_MKRWIFI1010

  // Arduino MKR WiFi 1010
  #include <SPI.h>
  #include <WiFiNINA.h>
  #include <Wire.h>
  #define TheWire Wire

  // I2C pins (SDA = D11, SCL = D12) are set by default so not defined here
  #define READY_PIN 0   // Arduino digital pin 0 connects to RDY
  #define L_INT_PIN 4   // Arduino digital pin 4 connects to LIT
  #define S_INT_PIN 5   // Arduino digital pin 5 connects to SIT
  /* In addition to these pins, the following I2C bus and power 
  connections must be made to the Metriful MS430: 
  Arduino pins GND, D12 (SCL), D11 (SDA) to MS430 pins GND, SCL, SDA
  Arduino pin VCC (3.3V) to MS430 pins VPU and VDD
  MS430 pin VIN is unused
   
  If a PPD42 particle sensor is used, connect the following:
  Arduino pin 5V to PPD42 pin 3
  Arduino pin GND to PPD42 pin 1
  PPD42 pin 4 to MS430 pin PRT
   
  If an SDS011 particle sensor is used, connect the following:
  Arduino pin 5V to SDS011 pin "5V"
  Arduino pin GND to SDS011 pin "GND"
  SDS011 pin "25um" to MS430 pin PRT
   
  For further details, see the readme and User Guide
  */
  
#elif defined ESP8266

  // NodeMCU ESP8266
  #include <ESP8266WiFi.h>
  #include <Wire.h>
  #define TheWire Wire

  #define SDA_PIN 5     // NodeMCU GPIO5 (labeled D1) connects to SDA
  #define SCL_PIN 4     // NodeMCU GPIO4 (labeled D2) connects to SCL
  #define READY_PIN 12  // NodeMCU GPIO12 (labeled D6) connects to RDY
  #define L_INT_PIN 0   // NodeMCU GPIO0 (labeled D3) connects to LIT
  #define S_INT_PIN 14  // NodeMCU GPIO14 (labeled D5) connects to SIT
  /* In addition to these pins, the following I2C bus and power 
  connections must be made to the Metriful MS430: 
  NodeMCU GND pin to MS430 pin GND
  NodeMCU pin 3V3 to MS430 pins VPU and VDD
  MS430 pin VIN is unused
   
  If a PPD42 particle sensor is used, also connect the following:
  NodeMCU pin Vin to PPD42 pin 3
  NodeMCU pin GND to PPD42 pin 1
  PPD42 pin 4 to MS430 pin PRT
  
  If an SDS011 particle sensor is used, connect the following:
  NodeMCU pin Vin to SDS011 pin "5V"
  NodeMCU pin GND to SDS011 pin "GND"
  SDS011 pin "25um" to MS430 pin PRT
   
  For further details, see the readme and User Guide
  */

#else
  #error ("Your development board is not directly supported - edit this file to define the correct input/output pins.")
#endif

#endif
