#  cycle_readout.py

#  Example code for using the Metriful board in cycle mode. 
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Continually measures and displays all environmental data in a 
#  repeating cycle. User can choose from a cycle time period 
#  of 3, 100, or 300 seconds.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit https://github.com/metriful/sensor

from sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# How often to read data (every 3, 100, 300 seconds)
cycle_period = CYCLE_PERIOD_3_S 

# Whether to read the particle data (set False if no PPD42 particle 
# sensor is connected, to avoid seeing spurious data).
get_particle_data = True

# How to print the data: If print_data_as_columns = true,
# data are columns of numbers, useful for transferring to a spreadsheet
# application. Otherwise, data are printed with explanatory labels and units.
print_data_as_columns = False

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Apply the chosen settings
if (get_particle_data):
  I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_ENABLE_REG, [ENABLED])
I2C_bus.write_i2c_block_data(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, [cycle_period])

#########################################################

print("Entering cycle mode and waiting for data. Press ctrl-c to exit.")

# Tell the Pi to monitor READY for a falling edge event (high-to-low voltage change)
GPIO.add_event_detect(READY_pin, GPIO.FALLING) 

# Tell the Metriful board to enter cycle mode
I2C_bus.write_byte(i2c_7bit_address, CYCLE_MODE_CMD)

while (True):

  # Wait for the next new data release, indicated by a falling edge on READY
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)

  # Now read and print all data

  # Air data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_DATA_READ, AIR_DATA_BYTES)
  air_data = extractAirData(raw_data)
  writeAirData(None, air_data, print_data_as_columns)

  # Air quality data
  # Note that the initial self-calibration of the air quality data 
  # takes a few minutes to complete. During this time the accuracy 
  # parameter is zero and the data values do not change.
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_QUALITY_DATA_READ, AIR_QUALITY_DATA_BYTES)
  air_quality_data = extractAirQualityData(raw_data)
  writeAirQualityData(None, air_quality_data, print_data_as_columns)

  # Light data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, LIGHT_DATA_READ, LIGHT_DATA_BYTES)
  light_data = extractLightData(raw_data)
  writeLightData(None, light_data, print_data_as_columns)

  # Sound data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, SOUND_DATA_READ, SOUND_DATA_BYTES)
  sound_data = extractSoundData(raw_data)
  writeSoundData(None, sound_data, print_data_as_columns)

  # Particle data
  # Note that this requires the connection of a PPD42 particle 
  # sensor (invalid values will be obtained if this sensor is not
  # present).
  # Also note that, due to the low pass filtering used, the 
  # particle data become valid after an initial stabilization 
  # period of approximately two minutes.
  if (get_particle_data):
    raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, PARTICLE_DATA_READ, PARTICLE_DATA_BYTES)
    particle_data = extractParticleData(raw_data)
    writeParticleData(None, particle_data, print_data_as_columns)
  
  if print_data_as_columns:
    print("")
  else:
    print("-------------------------------------------")
