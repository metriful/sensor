"""Example of logging data from the Metriful MS430, using a Raspberry Pi.

All environment data values are measured and saved as columns
of numbers in a text file (one row of data every three seconds).
This type of file can be imported into various graph and spreadsheet
applications. To prevent very large file sizes, a new file is
started every time it reaches a preset size limit.
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

import time
from datetime import datetime
import sensor_package.sensor_functions as sensor
import sensor_package.sensor_constants as const

#########################################################
# USER-EDITABLE SETTINGS

# Choose any combination of where to save data:
log_to_file = True
print_to_screen = True
# The log files are text files containing columns of data separated by spaces.

# Number of lines of data to log in each file before starting a new file
# (required if log_to_file == True), and which directory to save them in.
lines_per_file = 300
data_file_directory = "/home/pi/Desktop"

# How often to measure and read data (every 3, 100, or 300 seconds):
cycle_period = const.CYCLE_PERIOD_3_S

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = sensor.SensorHardwareSetup()

# Apply the chosen settings to the MS430
I2C_bus.write_i2c_block_data(
    sensor.i2c_7bit_address,
    const.PARTICLE_SENSOR_SELECT_REG, [sensor.PARTICLE_SENSOR])
I2C_bus.write_i2c_block_data(
    sensor.i2c_7bit_address, const.CYCLE_TIME_PERIOD_REG, [cycle_period])

#########################################################

if log_to_file:
    datafile = sensor.startNewDataFile(data_file_directory)
    data_file_lines = 0

print("Entering cycle mode and waiting for data. Press ctrl-c to exit.")

# Enter cycle mode
I2C_bus.write_byte(sensor.i2c_7bit_address, const.CYCLE_MODE_CMD)

while True:

    # Wait for the next new data release, indicated by a falling edge on READY
    while (not GPIO.event_detected(sensor.READY_pin)):
        time.sleep(0.05)

    # Air data
    # Choose output temperature unit (C or F) in sensor_functions.py
    air_data = sensor.get_air_data(I2C_bus)

    # Air quality data
    # The initial self-calibration of the air quality data may take several
    # minutes to complete. During this time the accuracy parameter is zero
    # and the data values are not valid.
    air_quality_data = sensor.get_air_quality_data(I2C_bus)

    # Light data
    light_data = sensor.get_light_data(I2C_bus)

    # Sound data
    sound_data = sensor.get_sound_data(I2C_bus)

    # Particle data
    # This requires the connection of a particulate sensor (zero/invalid
    # values will be obtained if this sensor is not present).
    # Specify your sensor model (PPD42 or SDS011) in sensor_functions.py
    # Also note that, due to the low pass filtering used, the
    # particle data become valid after an initial initialization
    # period of approximately one minute.
    particle_data = sensor.get_particle_data(I2C_bus, sensor.PARTICLE_SENSOR)

    if print_to_screen:
        # Display all data on screen as named quantities with units
        print("")
        print("------------------")
        sensor.writeAirData(None, air_data, False)
        sensor.writeAirQualityData(None, air_quality_data, False)
        sensor.writeLightData(None, light_data, False)
        sensor.writeSoundData(None, sound_data, False)
        if (sensor.PARTICLE_SENSOR != const.PARTICLE_SENSOR_OFF):
            sensor.writeParticleData(None, particle_data, False)

    if log_to_file:
        # Write the data as simple columns in a text file (without labels or
        # measurement units).
        # Start by writing date and time in columns 1-6
        datafile.write(datetime.now().strftime('%Y %m %d %H %M %S '))
        # Air data in columns 7-10
        sensor.writeAirData(datafile, air_data, True)
        # Air quality data in columns 11-14
        sensor.writeAirQualityData(datafile, air_quality_data, True)
        # Light data in columns 15 - 16
        sensor.writeLightData(datafile, light_data, True)
        # Sound data in columns 17 - 25
        sensor.writeSoundData(datafile, sound_data, True)
        if (sensor.PARTICLE_SENSOR != const.PARTICLE_SENSOR_OFF):
            # Particle data in columns 26 - 28
            sensor.writeParticleData(datafile, particle_data, True)
        datafile.write("\n")
        datafile.flush()
        data_file_lines += 1
        if data_file_lines >= lines_per_file:
            # Start a new log file to prevent very large files
            datafile.close()
            datafile = sensor.startNewDataFile(data_file_directory)
            data_file_lines = 0
