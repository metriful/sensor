/* 
   simple_read_T_H.ino

   Example code for using the Metriful MS430 to measure humidity 
   and temperature. 
   
   Measures and displays the humidity and temperature, demonstrating 
   the decoding of the signed temperature value. The data are also 
   read out and displayed a second time, using a “data category read” 
   of all Air measurement data. View the output in the Serial Monitor.

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

uint8_t receive_buffer[2] = {0};

void setup() {  
  // Initialize the host pins, set up the serial port and reset:
  SensorHardwareSetup(i2c_7bit_address); 
  
  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {
    yield();
  } 
  
  // Clear the global variable in preparation for waiting for READY assertion
  ready_assertion_event = false;
  
  // Initiate an on-demand data measurement
  TransmitI2C(i2c_7bit_address, ON_DEMAND_MEASURE_CMD, 0, 0);

  // Now wait for the ready signal before continuing
  while (!ready_assertion_event) {
    yield();
  }
    
  // We now know that data are ready to read.

  ////////////////////////////////////////////////////////////////////

  // HUMIDITY

  // Read the humidity value from the sensor board
  ReceiveI2C(i2c_7bit_address, H_READ, receive_buffer, H_BYTES);

  // Decode the humidity: the first byte is the integer part, the 
  // second byte is the fractional part to one decimal place.
  uint8_t humidity_integer = receive_buffer[0];
  uint8_t humidity_fraction = receive_buffer[1];
  // Print it: the units are percentage relative humidity.
  Serial.print("Humidity = ");
  Serial.print(humidity_integer);Serial.print(".");Serial.print(humidity_fraction);Serial.println(" %");

  ////////////////////////////////////////////////////////////////////

  // TEMPERATURE

  // Read the temperature value from the sensor board
  ReceiveI2C(i2c_7bit_address, T_READ, receive_buffer, T_BYTES);

  // Decode and print the temperature:

  // Find the positive magnitude of the integer part of the temperature by 
  // doing a bitwise AND of the first byte with TEMPERATURE_VALUE_MASK
  uint8_t temperature_positive_integer = receive_buffer[0] & TEMPERATURE_VALUE_MASK;

  // The second byte is the fractional part to one decimal place
  uint8_t temperature_fraction = receive_buffer[1];

  Serial.print("Temperature = ");
  // If the most-significant bit is set, the temperature is negative (below 0 C)
  if ((receive_buffer[0] & TEMPERATURE_SIGN_MASK) == 0) {
    // The bit is not set: celsius temperature is positive
    Serial.print("+");
  }
  else {
    // The bit is set: celsius temperature is negative
    Serial.print("-");
  }
  Serial.print(temperature_positive_integer);Serial.print(".");
  Serial.print(temperature_fraction);Serial.println(" C");

  ////////////////////////////////////////////////////////////////////

  // AIR DATA

  // Rather than reading individual data values as shown above, whole 
  // categories of data can be read in one I2C transaction 

  // Read all Air data in one transaction, interpreting the received bytes as an AirData_t struct:
  AirData_t airData = {0};
  ReceiveI2C(i2c_7bit_address, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);

  // Print the values over the serial port
  if (useFloatingPoint) {
    // Convert values where necessary to floating point representation:
    AirData_F_t airDataF = {0};
    convertAirDataF(&airData, &airDataF);
    printAirDataF(&airDataF);
  }
  else {
    // The data remain in integer representation 
    printAirData(&airData, false);
  }
}

void loop() {
  // There is no loop for this program.
}
