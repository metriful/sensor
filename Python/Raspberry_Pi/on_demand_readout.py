"""Example using the Metriful MS430 in "on-demand" mode, from a Raspberry Pi.

Repeatedly measures and displays all environment data, with a pause
of any chosen duration between measurements. Air quality data are
unavailable in this mode (instead use cycle_readout.py).

The measurements can be displayed as either labeled text, or as
simple columns of numbers.
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

import time
import sensor_package.sensor_functions as sensor
import sensor_package.sensor_constants as const

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
(GPIO, I2C_bus) = sensor.SensorHardwareSetup()

I2C_bus.write_i2c_block_data(
    sensor.i2c_7bit_address,
    const.PARTICLE_SENSOR_SELECT_REG, [sensor.PARTICLE_SENSOR])

#########################################################

while (True):

    time.sleep(pause_s)

    # Trigger a new measurement
    I2C_bus.write_byte(sensor.i2c_7bit_address, const.ON_DEMAND_MEASURE_CMD)

    # Wait for the next new data release, indicated by a falling edge on READY.
    # This will take 0.5 seconds.
    while (not GPIO.event_detected(sensor.READY_pin)):
        time.sleep(0.05)

    # Now read and print all data

    # Air data
    # Choose output temperature unit (C or F) in sensor_functions.py
    air_data = sensor.get_air_data(I2C_bus)
    sensor.writeAirData(None, air_data, print_data_as_columns)

    # Air quality data are not available with on demand measurements

    # Light data
    light_data = sensor.get_light_data(I2C_bus)
    sensor.writeLightData(None, light_data, print_data_as_columns)

    # Sound data
    sound_data = sensor.get_sound_data(I2C_bus)
    sensor.writeSoundData(None, sound_data, print_data_as_columns)

    # Particle data
    # This requires the connection of a particulate sensor (zero/invalid
    # values will be obtained if this sensor is not present).
    # Specify your sensor model (PPD42 or SDS011) in sensor_functions.py
    # Also note that, due to the low pass filtering used, the
    # particle data become valid after an initial initialization
    # period of approximately one minute.
    if (sensor.PARTICLE_SENSOR != const.PARTICLE_SENSOR_OFF):
        particle_data = sensor.get_particle_data(I2C_bus,
                                                 sensor.PARTICLE_SENSOR)
        sensor.writeParticleData(None, particle_data, print_data_as_columns)

    if print_data_as_columns:
        print("")
    else:
        print("-------------------------------------------")
