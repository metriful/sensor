#  simple_read_sound.py

#  Example code for using the Metriful MS430 to measure sound. 
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Demonstrates multiple ways of reading and displaying the sound data. 
#  View the output in the terminal. The other data can be measured
#  and displayed in a similar way.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

from sensor_package.sensor_functions import *

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

#########################################################

# Wait for the microphone signal to stabilize (takes approximately 1.5 seconds). 
# This only needs to be done once after the MS430 is powered-on or reset.
sleep(1.5);

#########################################################

# Initiate an on-demand data measurement
I2C_bus.write_byte(i2c_7bit_address, ON_DEMAND_MEASURE_CMD)

# Now wait for the ready signal (falling edge) before continuing
while (not GPIO.event_detected(READY_pin)):
  sleep(0.05)
  
# New data are now ready to read.

#########################################################

# There are multiple ways to read and display the data
  
  
# 1. Simplest way: use the example functions

# Read all sound data from the MS430 and convert to a Python dictionary
sound_data = get_sound_data(I2C_bus)

# Then print all the values onto the screen
writeSoundData(None, sound_data, False)

# Or you can use the dictionary values directly, for example:
print("The sound pressure level is: " + str(sound_data['SPL_dBA']) + " dBA")


# 2. Read the raw data bytes from the MS430 using an I2C function 
raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, SOUND_DATA_READ, SOUND_DATA_BYTES)

# Decode the values and return then as a Python dictionary
sound_data = extractSoundData(raw_data)

# Print the dictionary values in the same ways as before
writeSoundData(None, sound_data, False)
print("The sound pressure level is: " + str(sound_data['SPL_dBA']) + " dBA")


#########################################################

GPIO.cleanup()

