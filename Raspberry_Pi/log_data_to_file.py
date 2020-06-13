#  log_data_to_file.py
   
#  Example file data logging code for the Metriful board. 
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  All environmental data values are measured and saved as columns 
#  of numbers in a text file (one row of data every three seconds). 
#  This type of file can be imported into various graph and spreadsheet 
#  applications. To prevent very large file sizes, a new file is 
#  started every time it reaches a preset size limit.  

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit https://github.com/metriful/sensor

import datetime
from sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# Choose any combination of where to save data:
log_to_file = True
print_to_screen = True
# The log files are text files containing columns of data separated by spaces.

# Number of lines of data to log in each file before starting a new file
# (required if log_to_file == True), and which directory to save them in.
lines_per_file = 300
data_file_directory = "/home/pi"

# How often to measure and read data (every 3, 100, 300 seconds):
cycle_period = CYCLE_PERIOD_3_S 

# Whether to read the particle data (set False if no PPD42 particle 
# sensor is connected, to avoid seeing spurious data).
get_particle_data = True

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Apply the chosen settings to the Metriful board
if (get_particle_data):
  I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_ENABLE_REG, [ENABLED])
I2C_bus.write_i2c_block_data(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, [cycle_period])

#########################################################

if log_to_file:
  datafile = startNewDataFile(data_file_directory)
  data_file_lines = 0

print("Entering cycle mode and waiting for data. Press ctrl-c to exit.")

# Tell the Pi to monitor READY for a falling edge event (high-to-low voltage change)
GPIO.add_event_detect(READY_pin, GPIO.FALLING) 

# Tell Metriful to enter cycle mode
I2C_bus.write_byte(i2c_7bit_address, CYCLE_MODE_CMD)

while (True):

  # Wait for the next new data release, indicated by a falling edge on READY
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)

  # Air data:
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_DATA_READ, AIR_DATA_BYTES)
  air_data = extractAirData(raw_data)
  
  # Air quality data
  # Note that the initial self-calibration of the air quality data 
  # takes a few minutes to complete. During this time the accuracy 
  # parameter is zero and the data values do not change.
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_QUALITY_DATA_READ, AIR_QUALITY_DATA_BYTES)
  air_quality_data = extractAirQualityData(raw_data)
    
  # Light data:
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, LIGHT_DATA_READ, LIGHT_DATA_BYTES)
  light_data = extractLightData(raw_data)
  
  # Sound data:
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, SOUND_DATA_READ, SOUND_DATA_BYTES)
  sound_data = extractSoundData(raw_data)
    
  # Particle data
  # Note that this requires the connection of a PPD42 particle 
  # sensor (invalid values will be obtained if this sensor is not
  # present).
  # Also note that, due to the low pass filtering used, the 
  # particle data become valid after an initial stabilization 
  # period of approximately two minutes.
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, PARTICLE_DATA_READ, PARTICLE_DATA_BYTES)
  particle_data = extractParticleData(raw_data)
    
  if (print_to_screen):
    # Display all data on screen as named quantities with units
    print("")
    print("------------------");
    writeAirData(None, air_data, False)
    print("------------------");
    writeAirQualityData(None, air_quality_data, False)
    print("------------------");
    writeLightData(None, light_data, False)
    print("------------------");
    writeSoundData(None, sound_data, False)
    print("------------------");
    if (get_particle_data):
      writeParticleData(None, particle_data, False)
      print("------------------");
      

  if (log_to_file):
    # Write the data as simple columns in a text file (without labels or
    # measurement units).
    # Start by writing date and time in columns 1-6
    datafile.write(datetime.datetime.now().strftime('%Y %m %d %H %M %S '))
    # Air data in columns 7-10
    writeAirData(datafile, air_data, True)
    # Air quality data in columns 11-14
    writeAirQualityData(datafile, air_quality_data, True)
    # Light data in columns 15 - 16
    writeLightData(datafile, light_data, True)
    # Sound data in columns 17 - 25
    writeSoundData(datafile, sound_data, True)
    if (get_particle_data):
      # Particle data in columns 26 - 27
      writeParticleData(datafile, particle_data, True)
    datafile.write("\n")
    datafile.flush()
    data_file_lines+=1
    if (data_file_lines >= lines_per_file):
      # Start a new log file to prevent very large files
      datafile.close()
      datafile = startNewDataFile(data_file_directory)
      data_file_lines = 0



