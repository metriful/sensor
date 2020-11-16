/* 
   host_pin_definitions.h

   This file defines which host pins are used to interface to the 
   Metriful MS430 board. The relevant file section is selected 
   automatically when the board is chosen in the Arduino IDE.

   More detail is provided in the readme and User Guide.

   This file provides settings for the following host systems:
   * Arduino Uno
   * Arduino Nano 33 IoT
   * Arduino Nano
   * Arduino MKR WiFi 1010
   * ESP8266 (tested on NodeMCU and Wemos D1 Mini - other boards may require changes)
   * ESP32 (tested on DOIT ESP32 DEVKIT V1 - other boards may require changes)

   The Metriful MS430 is compatible with many more development boards
   than those listed. You can use this file as a guide to define the
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

  #define ISR_ATTRIBUTE

  #define READY_PIN 2   // Arduino digital pin 2 connects to RDY
  #define L_INT_PIN 4   // Arduino digital pin 4 connects to LIT
  #define S_INT_PIN 7   // Arduino digital pin 7 connects to SIT
  /* Also make the following connections:
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
  */

#elif defined ARDUINO_SAMD_NANO_33_IOT

  // Arduino Nano 33 IoT
  
  #include <SPI.h>
  #include <WiFiNINA.h>
  #define HAS_WIFI
  #define ISR_ATTRIBUTE

  #define READY_PIN 11  // Arduino pin D11 connects to RDY
  #define L_INT_PIN A1  // Arduino pin A1 connects to LIT
  #define S_INT_PIN A2  // Arduino pin A2 connects to SIT
  /* Also make the following connections:
  Arduino pin GND to MS430 pin GND
  Arduino pin 3.3V to MS430 pins VPU and VDD
  Arduino pin A5 to MS430 pin SCL
  Arduino pin A4 to MS430 pin SDA
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
  */

#elif defined ARDUINO_AVR_NANO

  // Arduino Nano

  #define ISR_ATTRIBUTE

  #define READY_PIN 2   // Arduino pin D2 connects to RDY
  #define L_INT_PIN 4   // Arduino pin D4 connects to LIT
  #define S_INT_PIN 7   // Arduino pin D7 connects to SIT
  /* Also make the following connections:
  Arduino pin GND to MS430 pin GND
  Arduino pin A5 (SCL) to MS430 pin SCL
  Arduino pin A4 (SDA) to MS430 pin SDA
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
  */

#elif defined ARDUINO_SAMD_MKRWIFI1010

  // Arduino MKR WiFi 1010

  #include <SPI.h>
  #include <WiFiNINA.h>
  #define HAS_WIFI
  #define ISR_ATTRIBUTE

  #define READY_PIN 0   // Arduino digital pin 0 connects to RDY
  #define L_INT_PIN 4   // Arduino digital pin 4 connects to LIT
  #define S_INT_PIN 5   // Arduino digital pin 5 connects to SIT
  /* Also make the following connections: 
  Arduino pin GND to MS430 pin GND
  Arduino pin D12 (SCL) to MS430 pin SCL
  Arduino pin D11 (SDA) to MS430 pin SDA
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
  */

#elif defined ESP8266

  // The examples have been tested on NodeMCU and Wemos D1 Mini.
  // Other ESP8266 boards may require changes.

  #include <ESP8266WiFi.h>
  #define HAS_WIFI
  #define ISR_ATTRIBUTE ICACHE_RAM_ATTR

  #define SDA_PIN 5     // GPIO5  (labeled D1) connects to SDA
  #define SCL_PIN 4     // GPIO4  (labeled D2) connects to SCL
  #define READY_PIN 12  // GPIO12 (labeled D6) connects to RDY
  #define L_INT_PIN 0   // GPIO0  (labeled D3) connects to LIT
  #define S_INT_PIN 14  // GPIO14 (labeled D5) connects to SIT
  /* Also make the following connections:
  ESP8266 pin GND to MS430 pin GND
  ESP8266 pin 3V3 to MS430 pins VPU and VDD
  MS430 pin VIN is unused
   
  If a PPD42 particle sensor is used, also connect the following:
  ESP8266 pin Vin (may be labeled Vin or 5V or VU) to PPD42 pin 3
  ESP8266 pin GND to PPD42 pin 1
  PPD42 pin 4 to MS430 pin PRT
  
  If an SDS011 particle sensor is used, connect the following:
  ESP8266 pin Vin (may be labeled Vin or 5V or VU) to SDS011 pin "5V"
  ESP8266 pin GND to SDS011 pin "GND"
  SDS011 pin "25um" to MS430 pin PRT
  */

#elif defined ESP32

  // The examples have been tested on DOIT ESP32 DEVKIT V1 development board.
  // Other ESP32 boards may require changes.

  #include <WiFi.h>
  #define HAS_WIFI
  #define ISR_ATTRIBUTE IRAM_ATTR

  #define READY_PIN 23  // Pin D23 connects to RDY
  #define L_INT_PIN 18  // Pin D18 connects to LIT
  #define S_INT_PIN 19  // Pin D19 connects to SIT
  /* Also make the following connections:
  ESP32 pin D21 to MS430 pin SDA
  ESP32 pin D22 to MS430 pin SCL
  ESP32 pin GND to MS430 pin GND
  ESP32 pin 3V3 to MS430 pins VPU and VDD
  MS430 pin VIN is unused

  If a PPD42 particle sensor is used, also connect the following:
  ESP32 pin Vin to PPD42 pin 3
  ESP32 pin GND to PPD42 pin 1
  PPD42 pin 4 to MS430 pin PRT

  If an SDS011 particle sensor is used, connect the following:
  ESP32 pin Vin to SDS011 pin "5V"
  ESP32 pin GND to SDS011 pin "GND"
  SDS011 pin "25um" to MS430 pin PRT
  */

#else
  #error ("Your development board is not directly supported")
  // Please make a new section in this file to define the correct input/output pins
#endif

#endif
