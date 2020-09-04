#  on_demand_readout.py

#  Example code for using the Metriful MS430 in "on-demand" mode.
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Repeatedly measures and displays all environment data, with a pause
#  between measurements. Air quality data are unavailable in this mode 
#  (instead see cycle_readout.py).

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

from sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# Pause (in seconds) between data measurements (note that the
# measurement itself takes 0.5 seconds)
pause_s = 3.5
# Choosing a pause of less than 2 seconds will cause inaccurate 
# temperature, humidity and particle data.

# Which particle sensor, if any, is attached 
# (PARTICLE_SENSOR_X with X = PPD42, SDS011, or OFF)
particleSensor = PARTICLE_SENSOR_OFF

# How to print the data: If print_data_as_columns = True,
# data are columns of numbers, useful to copy/paste to a spreadsheet
# application. Otherwise, data are printed with explanatory labels and units.
print_data_as_columns = False

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Apply the chosen settings
if (particleSensor != PARTICLE_SENSOR_OFF):
  I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [particleSensor])

#########################################################

while (True):

  sleep(pause_s)
  
  # Trigger a new measurement
  I2C_bus.write_byte(i2c_7bit_address, ON_DEMAND_MEASURE_CMD)

  # Wait for the next new data release, indicated by a falling edge on READY
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)

  # Now read and print all data

  # Air data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_DATA_READ, AIR_DATA_BYTES)
  air_data = extractAirData(raw_data)
  writeAirData(None, air_data, print_data_as_columns)

  # Air quality data are not available with on demand measurements

  # Light data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, LIGHT_DATA_READ, LIGHT_DATA_BYTES)
  light_data = extractLightData(raw_data)
  writeLightData(None, light_data, print_data_as_columns)

  # Sound data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, SOUND_DATA_READ, SOUND_DATA_BYTES)
  sound_data = extractSoundData(raw_data)
  writeSoundData(None, sound_data, print_data_as_columns)

  # Particle data
  # This requires the connection of a particulate sensor (invalid 
  # values will be obtained if this sensor is not present).
  # Also note that, due to the low pass filtering used, the 
  # particle data become valid after an initial initialization 
  # period of approximately one minute.
  if (particleSensor != PARTICLE_SENSOR_OFF):
    raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, PARTICLE_DATA_READ, PARTICLE_DATA_BYTES)
    particle_data = extractParticleData(raw_data, particleSensor)
    writeParticleData(None, particle_data, print_data_as_columns)
  
  if print_data_as_columns:
    print("")
  else:
    print("-------------------------------------------")
