#  on_demand_readout.py

#  Example code for using the Metriful MS430 in "on-demand" mode.
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Repeatedly measures and displays all environment data, with a pause
#  of any chosen duration between measurements. Air quality data are 
#  unavailable in this mode (instead use cycle_readout.py).

#  The measurements can be displayed as either labeled text, or as 
#  simple columns of numbers.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

from sensor_package.sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# Pause (in seconds) between data measurements (note that the
# measurement itself takes 0.5 seconds)
pause_s = 3.5
# Choosing a pause of less than 2 seconds will cause inaccurate 
# temperature, humidity and particle data.

# How to print the data: If print_data_as_columns = True,
# data are columns of numbers, useful to copy/paste to a spreadsheet
# application. Otherwise, data are printed with explanatory labels and units.
print_data_as_columns = False

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [PARTICLE_SENSOR])

#########################################################

while (True):

  sleep(pause_s)
  
  # Trigger a new measurement
  I2C_bus.write_byte(i2c_7bit_address, ON_DEMAND_MEASURE_CMD)

  # Wait for the next new data release, indicated by a falling edge on READY.
  # This will take 0.5 seconds.
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)

  # Now read and print all data

  # Air data
  # Choose output temperature unit (C or F) in sensor_functions.py
  air_data = get_air_data(I2C_bus)
  writeAirData(None, air_data, print_data_as_columns)

  # Air quality data are not available with on demand measurements

  # Light data
  light_data = get_light_data(I2C_bus)
  writeLightData(None, light_data, print_data_as_columns)

  # Sound data
  sound_data = get_sound_data(I2C_bus)
  writeSoundData(None, sound_data, print_data_as_columns)

  # Particle data
  # This requires the connection of a particulate sensor (zero/invalid 
  # values will be obtained if this sensor is not present).
  # Specify your sensor model (PPD42 or SDS011) in sensor_functions.py
  # Also note that, due to the low pass filtering used, the 
  # particle data become valid after an initial initialization 
  # period of approximately one minute.
  if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF):
    particle_data = get_particle_data(I2C_bus, PARTICLE_SENSOR)
    writeParticleData(None, particle_data, print_data_as_columns)
  
  if print_data_as_columns:
    print("")
  else:
    print("-------------------------------------------")
