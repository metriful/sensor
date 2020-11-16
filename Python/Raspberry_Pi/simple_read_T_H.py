#  simple_read_T_H.py

#  Example code for using the Metriful MS430 to measure humidity and
#  temperature. 
#  This example is designed to run with Python 3 on a Raspberry Pi.

#  Demonstrates multiple ways of reading and displaying the temperature 
#  and humidity data. View the output in the terminal. The other data
#  can be measured and displayed in a similar way.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

from sensor_package.sensor_functions import *

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Initiate an on-demand data measurement
I2C_bus.write_byte(i2c_7bit_address, ON_DEMAND_MEASURE_CMD)

# Now wait for the ready signal (falling edge) before continuing
while (not GPIO.event_detected(READY_pin)):
  sleep(0.05)
  
# New data are now ready to read.

#########################################################

# There are multiple ways to read and display the data
  
  
# 1. Simplest way: use the example functions

# Read all the "air data" from the MS430. This includes temperature and 
# humidity as well as pressure and gas sensor data. Return the data as
# a data dictionary.
air_data = get_air_data(I2C_bus)

# Then print all the values onto the screen
writeAirData(None, air_data, False)

# Or you can use the values directly
print("The temperature is: {:.1f} ".format(air_data['T_C']) + air_data['C_unit'])
print("The humidity is: {:.1f} %".format(air_data['H_pc']))

# Temperature can also be output in Fahrenheit units
print("The temperature is: {:.1f} ".format(air_data['T_F']) + air_data['F_unit'])

# The default temperature unit can be set in sensor_functions.py and used like:
print("The temperature is: {:.1f} ".format(air_data['T']) + air_data['T_unit'])

print("-----------------------------")

# 2. Advanced: read and decode only the humidity value

# Get the data from the MS430
raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, H_READ, H_BYTES)

# Decode the humidity: the first received byte is the integer part, the 
# second byte is the fractional part (to one decimal place).
humidity = raw_data[0] + float(raw_data[1])/10.0

# Print it: the units are percentage relative humidity.
print("Humidity = {:.1f} %".format(humidity))

print("-----------------------------")

# 3. Advanced: read and decode only the temperature value (Celsius)

# Get the data from the MS430
raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, T_READ, T_BYTES)

# Find the positive magnitude of the integer part of the temperature by 
# doing a bitwise AND of the first received byte with TEMPERATURE_VALUE_MASK
temp_positive_integer = raw_data[0] & TEMPERATURE_VALUE_MASK

# The second received byte is the fractional part to one decimal place
temp_fraction = raw_data[1]

# Combine to form a positive floating point number:
temperature = temp_positive_integer + float(temp_fraction)/10.0

# Now find the sign of the temperature: if the most-significant bit of 
# the first byte is a 1, the temperature is negative (below 0 C)
if ((raw_data[0] & TEMPERATURE_SIGN_MASK) != 0):
  # The bit is a 1: temperature is negative
  temperature = -temperature

# Print the temperature: the units are degrees Celsius.
print("Temperature = {:.1f} ".format(temperature) + CELSIUS_SYMBOL)

#########################################################

GPIO.cleanup()

