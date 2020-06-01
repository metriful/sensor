#  simple_read_sound.py

#  Example code for using the Sense board to measure sound. 
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Waits for microphone initialization, then measures and displays 
#  the sound data.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit https://github.com/metriful/sense

from time import sleep
from Sense_functions import *

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SenseHardwareSetup()

#########################################################

# Wait for the microphone to stabilize (takes approximately 1.5 seconds). 
# The microphone uses a filter which should be allowed to settle before 
# sound data are measured. 
# This only needs to be done once after Sense is powered-on or reset. 

mic_stable = False
while (not mic_stable):
  # The sound_stable register is set to a non-zero value when the microphone has initialized
  byte_value = I2C_bus.read_i2c_block_data(i2c_7bit_address, SOUND_STABLE_READ, SOUND_STABLE_BYTES)
  mic_stable = (byte_value[0] != 0)
  sleep(0.05)

#########################################################

# Tell the Pi to monitor READY for a falling edge event (high-to-low voltage change)
GPIO.add_event_detect(READY_pin, GPIO.FALLING) 

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
# writeSoundData(None, sound_data, False)

#########################################################

GPIO.cleanup()

