"""Example of using the Metriful MS430 to measure sound, from a Raspberry Pi.

Demonstrates multiple ways of reading and displaying the sound data.
View the output in the terminal. The other data can be measured
and displayed in a similar way.
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

#########################################################

# Wait for the microphone signal to stabilize (takes approximately
# 1.5 seconds). This only needs to be done once after the
# MS430 is powered-on or reset.
time.sleep(1.5)

#########################################################

# Initiate an on-demand data measurement
I2C_bus.write_byte(sensor.i2c_7bit_address, const.ON_DEMAND_MEASURE_CMD)

# Now wait for the ready signal (falling edge) before continuing
while (not GPIO.event_detected(sensor.READY_pin)):
    time.sleep(0.05)

# New data are now ready to read; this can be done in multiple ways:


# 1. Simplest way: use the example functions

# Read all sound data from the MS430 and convert to a Python dictionary
sound_data = sensor.get_sound_data(I2C_bus)

# Then print all the values onto the screen
sensor.writeSoundData(None, sound_data, False)

# Or you can use the dictionary values directly, for example:
print("The sound pressure level is: " + str(sound_data['SPL_dBA']) + " dBA")


# 2. Read the raw data bytes from the MS430 using an I2C function
raw_data = I2C_bus.read_i2c_block_data(
    sensor.i2c_7bit_address, const.SOUND_DATA_READ, const.SOUND_DATA_BYTES)

# Decode the values and return then as a Python dictionary
sound_data = sensor.extractSoundData(raw_data)

# Print the dictionary values in the same ways as before
sensor.writeSoundData(None, sound_data, False)
print("The sound pressure level is: " + str(sound_data['SPL_dBA']) + " dBA")

#########################################################

GPIO.cleanup()
