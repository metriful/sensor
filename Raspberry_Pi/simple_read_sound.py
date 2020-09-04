#  simple_read_sound.py

#  Example code for using the Metriful MS430 to measure sound. 
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Measures and displays the sound data once. 

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

from time import sleep
from sensor_functions import *

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
  
# We now know that newly measured data are ready to read.

#########################################################

# SOUND DATA

# Read all sound data in one transaction
raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, SOUND_DATA_READ, SOUND_DATA_BYTES)

# Use the example function to decode the values and return then as a Python dictionary
sound_data = extractSoundData(raw_data)

# Print the values obtained
print("A-weighted sound pressure level = {:.1f} dBA".format(sound_data['SPL_dBA']))
for i in range(0,SOUND_FREQ_BANDS):
  print("Frequency band " + str(i+1) + " (" + str(sound_band_mids_Hz[i]) 
                          + " Hz) SPL = {:.1f} dB".format(sound_data['SPL_bands_dB'][i]))
print("Peak sound amplitude = {:.2f} mPa".format(sound_data['peak_amp_mPa']))

# Or just use the following function for printing:
writeSoundData(None, sound_data, False)

#########################################################

GPIO.cleanup()

