"""Example of measurements using the Metriful MS430 from a Raspberry Pi.

Demonstrates multiple ways of reading and displaying the temperature
and humidity data. View the output in the terminal. The other data
can be measured and displayed in a similar way.
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

import time
import sensor_package.sensor_functions as sensor
import sensor_package.sensor_constants as const

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = sensor.SensorHardwareSetup()

# Initiate an on-demand data measurement
I2C_bus.write_byte(sensor.i2c_7bit_address, const.ON_DEMAND_MEASURE_CMD)

# Now wait for the ready signal (falling edge) before continuing
while (not GPIO.event_detected(sensor.READY_pin)):
    time.sleep(0.05)

# New data are now ready to read; this can be done in multiple ways:


# 1. Simplest way: use the example functions

# Read all the "air data" from the MS430. This includes temperature and
# humidity as well as pressure and gas sensor data. Return the data as
# a data dictionary.
air_data = sensor.get_air_data(I2C_bus)

# Then print all the values onto the screen
sensor.writeAirData(None, air_data, False)

# Or you can use the values directly
print(f"The temperature is: {air_data['T_C']:.1f} {air_data['C_unit']}")
print(f"The humidity is: {air_data['H_pc']:.1f} %")

# Temperature can also be output in Fahrenheit units
print(f"The temperature is: {air_data['T_F']:.1f} {air_data['F_unit']}")

# The default temperature unit can be set in sensor_functions.py and used like:
print(f"The temperature is: {air_data['T']:.1f} {air_data['T_unit']}")

print("-----------------------------")

# 2. Advanced: read and decode only the humidity value

# Get the data from the MS430
raw_data = I2C_bus.read_i2c_block_data(sensor.i2c_7bit_address,
                                       const.H_READ, const.H_BYTES)

# Decode the humidity: the first received byte is the integer part, the
# second byte is the fractional part (to one decimal place).
humidity = raw_data[0] + float(raw_data[1])/10.0

# Print it: the units are percentage relative humidity.
print(f"Humidity = {humidity:.1f} %")

print("-----------------------------")

# 3. Advanced: read and decode only the temperature value (Celsius)

# Get the data from the MS430
raw_data = I2C_bus.read_i2c_block_data(sensor.i2c_7bit_address,
                                       const.T_READ, const.T_BYTES)

# Find the positive magnitude of the integer part of the temperature by
# doing a bitwise AND of the first received byte with TEMPERATURE_VALUE_MASK
temp_positive_integer = raw_data[0] & const.TEMPERATURE_VALUE_MASK

# The second received byte is the fractional part to one decimal place
temp_fraction = raw_data[1]

# Combine to form a positive floating point number:
temperature = temp_positive_integer + float(temp_fraction)/10.0

# Now find the sign of the temperature: if the most-significant bit of
# the first byte is a 1, the temperature is negative (below 0 C)
if (raw_data[0] & const.TEMPERATURE_SIGN_MASK) != 0:
    # The bit is a 1: temperature is negative
    temperature = -temperature

# Print the temperature: the units are degrees Celsius.
print(f"Temperature = {temperature:.1f} {const.CELSIUS_SYMBOL}")

#########################################################

GPIO.cleanup()
