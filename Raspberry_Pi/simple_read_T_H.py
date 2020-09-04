#  simple_read_T_H.py

#  Example code for using the Metriful MS430 to measure humidity and
#  temperature. 
#  This example is designed to run with Python 3 on a Raspberry Pi.

#  Measures and displays the humidity and temperature, demonstrating 
#  the decoding of the signed temperature value. The data are also 
#  read out and displayed a second time, using a "data category read"
#  of all Air measurement data.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

from sensor_functions import *

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Initiate an on-demand data measurement
I2C_bus.write_byte(i2c_7bit_address, ON_DEMAND_MEASURE_CMD)

# Now wait for the ready signal (falling edge) before continuing
while (not GPIO.event_detected(READY_pin)):
  sleep(0.05)
  
# We now know that newly measured data are ready to read.

#########################################################

# HUMIDITY

# Read the humidity value from the Metriful board
raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, H_READ, H_BYTES)

# Decode the humidity: the first byte is the integer part, the 
# second byte is the fractional part to one decimal place.
humidity_integer = raw_data[0]
humidity_fraction = raw_data[1]

# Print it: the units are percentage relative humidity.
print("Humidity = " + str(humidity_integer) + "." + str(humidity_fraction) + " %")

#########################################################

# TEMPERATURE

# Read the temperature value from the Metriful board
raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, T_READ, T_BYTES)

# Decode and print the temperature:

# Find the positive magnitude of the integer part of the temperature by 
# doing a bitwise AND of the first byte with TEMPERATURE_VALUE_MASK
temperature_positive_integer = raw_data[0] & TEMPERATURE_VALUE_MASK

# The second byte is the fractional part to one decimal place
temperature_fraction = raw_data[1]

# If the most-significant bit is set, the temperature is negative (below 0 C)
if ((raw_data[0] & TEMPERATURE_SIGN_MASK) == 0):
  # Bit not set: temperature is positive
  sign_string = "+"
else:
  # Bit is set: temperature is negative
  sign_string = "-"

# Print the temperature: the units are degrees Celsius.
print("Temperature = " + sign_string + str(temperature_positive_integer) + "." 
                       + str(temperature_fraction) + " C")

#########################################################

# AIR DATA

# Rather than reading individual data values as shown above, whole 
# categories of data can be read in one I2C transaction 

# Read all Air data in one transaction
raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_DATA_READ, AIR_DATA_BYTES)

# Use the example function to decode the values and return then as a Python dictionary
air_data = extractAirData(raw_data)

# Print the values obtained
print("Temperature = {:.1f} C".format(air_data['T_C']))
print("Pressure = " + str(air_data['P_Pa']) + " Pa")
print("Humidity = {:.1f} %".format(air_data['H_pc']))
print("Gas sensor resistance = " + str(air_data['G_ohm']) + " ohm")

# Or just use the following function for printing:
writeAirData(None, air_data, False)

#########################################################

GPIO.cleanup()

