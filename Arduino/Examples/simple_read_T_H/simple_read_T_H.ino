/* 
   simple_read_T_H.ino

   Example code for using the Metriful MS430 to measure humidity 
   and temperature. 
   
   Demonstrates multiple ways of reading and displaying the temperature 
   and humidity data. View the output in the Serial Monitor. The other 
   data can be measured and displayed in a similar way.

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
  
  // Clear the global variable in preparation for waiting for READY assertion
  ready_assertion_event = false;
  
  // Initiate an on-demand data measurement
  TransmitI2C(I2C_ADDRESS, ON_DEMAND_MEASURE_CMD, 0, 0);

  // Now wait for the ready signal before continuing
  while (!ready_assertion_event) {
    yield();
  }

  // We know that new data are ready to read.

  ////////////////////////////////////////////////////////////////////

  // There are different ways to read and display the data

  // 1. Simplest way: use the example float (_F) functions

  // Read the "air data" from the MS430. This includes temperature and 
  // humidity as well as pressure and gas sensor data.
  AirData_F_t airDataF = getAirDataF(I2C_ADDRESS);

  // Print all of the air measurements to the serial monitor
  printAirDataF(&airDataF);
  // Fahrenheit temperature is printed if USE_FAHRENHEIT is defined 
  // in "Metriful_sensor.h"

  Serial.println("-----------------------------");


  // 2. After reading from the MS430, you can also access and print the 
  // float data directly from the struct:
  Serial.print("The temperature is: ");
  Serial.print(airDataF.T_C, 1);   // print to 1 decimal place
  Serial.println(" " CELSIUS_SYMBOL);
  
  // Optional: convert to Fahrenheit
  float temperature_F = convertCtoF(airDataF.T_C);

  Serial.print("The temperature is: ");
  Serial.print(temperature_F, 1);   // print to 1 decimal place
  Serial.println(" " FAHRENHEIT_SYMBOL);

  Serial.println("-----------------------------");
  
  
  // 3. If host resources are limited, avoid using floating point and 
  // instead use the integer versions (without "F" in the name)
  AirData_t airData = getAirData(I2C_ADDRESS);
  
  // Print to the serial monitor
  printAirData(&airData, false);
  // If the second argument is "true", data are printed as columns.
  // Fahrenheit temperature is printed if USE_FAHRENHEIT is defined 
  // in "Metriful_sensor.h"
  
  Serial.println("-----------------------------");
  
  
  // 4. Access and print integer data directly from the struct:
  Serial.print("The humidity is: ");
  Serial.print(airData.H_pc_int);    // the integer part of the value
  Serial.print(".");                 // the decimal point
  Serial.print(airData.H_pc_fr_1dp); // the fractional part (1 decimal place) 
  Serial.println(" %");

  Serial.println("-----------------------------");


  // 5. Advanced: read and decode only the humidity value from the MS430

  // Read the raw humidity data 
  uint8_t receive_buffer[2] = {0};
  ReceiveI2C(I2C_ADDRESS, H_READ, receive_buffer, H_BYTES);

  // Decode the humidity: the first received byte is the integer part, the 
  // second received byte is the fractional part to one decimal place.
  uint8_t humidity_integer = receive_buffer[0];
  uint8_t humidity_fraction = receive_buffer[1];
  // Print it: the units are percentage relative humidity.
  Serial.print("Humidity = ");
  Serial.print(humidity_integer);
  Serial.print(".");
  Serial.print(humidity_fraction);
  Serial.println(" %");
  
  Serial.println("-----------------------------");
  

  // 6. Advanced: read and decode only the temperature value from the MS430

  // Read the raw temperature data
  ReceiveI2C(I2C_ADDRESS, T_READ, receive_buffer, T_BYTES);

  // The temperature is encoded differently to allow negative values

  // Find the positive magnitude of the integer part of the temperature 
  // by doing a bitwise AND of the first received byte with TEMPERATURE_VALUE_MASK
  uint8_t temperature_positive_integer = receive_buffer[0] & TEMPERATURE_VALUE_MASK;

  // The second received byte is the fractional part to one decimal place
  uint8_t temperature_fraction = receive_buffer[1];

  Serial.print("Temperature = ");
  // If the most-significant bit of the first byte is a 1, the temperature 
  // is negative (below 0 C), otherwise it is positive
  if ((receive_buffer[0] & TEMPERATURE_SIGN_MASK) != 0) {
    // The bit is a 1: celsius temperature is negative
    Serial.print("-");
  }
  Serial.print(temperature_positive_integer);
  Serial.print(".");
  Serial.print(temperature_fraction);
  Serial.println(" " CELSIUS_SYMBOL);

}

void loop() {
  // There is no loop for this program.
}
