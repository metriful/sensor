"""Functions and settings for use on Raspberry Pi.

Choose the preferred temperature measurement unit (Celsius or
Fahrenheit) in this file, and select the optional particle sensor.
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

import sys
from time import sleep
from datetime import datetime
import RPi.GPIO as GPIO
import smbus
import os
from . import sensor_constants as const

#############################################################################

# Choose to display output temperatures in Fahrenheit (instead of Celsius):
USE_FAHRENHEIT = False

# Specify which particle sensor is connected:
PARTICLE_SENSOR = const.PARTICLE_SENSOR_OFF
# The possibilities are:
#    PARTICLE_SENSOR_PPD42    for the Shinyei PPD42
#    PARTICLE_SENSOR_SDS011   for the Nova SDS011 (recommended)
#    PARTICLE_SENSOR_OFF      if no sensor is connected

#############################################################################

# GPIO (input/output) header pin numbers: these must match the hardware wiring.
# The light and sound interrupt pins are not used in all examples.
light_int_pin = 7  # Raspberry Pi pin 7 connects to LIT
sound_int_pin = 8  # Raspberry Pi pin 8 connects to SIT
READY_pin = 11    # Raspberry Pi pin 11 connects to RDY
# In addition to these GPIO pins, the following I2C and power
# connections must be made:
# Raspberry Pi pin 3 to SDA
# Raspberry Pi pin 5 to SCL
# Raspberry Pi pin 6 to GND (0 V)
# Raspberry Pi pin 1 to VDD and VPU (3.3 V)
# MS430 pin VIN is not used.
#
# If a PPD42 particle sensor is used, connect the following:
# Raspberry Pi pin 2 to PPD42 pin 3
# Raspberry Pi pin 9 to PPD42 pin 1
# PPD42 pin 4 to MS430 pin PRT
#
# If an SDS011 particle sensor is used, connect the following:
# Raspberry Pi pin 2 to SDS011 pin "5V"
# Raspberry Pi pin 9 to SDS011 pin "GND"
# SDS011 pin "25um" to MS430 pin PRT
#
# For further details, see the readme and User Guide

# The I2C address of the Metriful MS430 board
i2c_7bit_address = const.I2C_ADDR_7BIT_SB_OPEN

#############################################################################

def SensorHardwareSetup():
    """Set up the Raspberry Pi GPIO."""
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(READY_pin, GPIO.IN)
    GPIO.setup(light_int_pin, GPIO.IN)
    GPIO.setup(sound_int_pin, GPIO.IN)

    # Initialize the I2C communications bus object
    I2C_bus = smbus.SMBus(1)  # Port 1 is the default for I2C on Raspberry Pi

    # Wait for the MS430 to finish power-on initialization:
    while (GPIO.input(READY_pin) == 1):
        sleep(0.05)

    # Reset MS430 to clear any previous state:
    I2C_bus.write_byte(i2c_7bit_address, const.RESET_CMD)
    sleep(0.005)

    # Wait for reset completion and entry to standby mode
    while (GPIO.input(READY_pin) == 1):
        sleep(0.05)

    # Tell the Pi to monitor READY for a falling edge
    # event (high-to-low voltage change)
    GPIO.add_event_detect(READY_pin, GPIO.FALLING)

    return (GPIO, I2C_bus)

#############################################################################

# "extract*Data" are functions to convert the raw data bytes (received over
# I2C) into Python dictionaries containing the environmental data values.


def extractAirData(rawData):
    if len(rawData) != const.AIR_DATA_BYTES:
        raise ValueError('Incorrect number of Air Data bytes')
    air_data = {'T_C': 0, 'P_Pa': 0, 'H_pc': 0, 'G_ohm': 0}
    air_data['T_C'] = ((rawData[0] & const.TEMPERATURE_VALUE_MASK)
                       + (float(rawData[1])/10.0))
    if (rawData[0] & const.TEMPERATURE_SIGN_MASK) != 0:
        # the most-significant bit is set, indicating that the
        # temperature is negative
        air_data['T_C'] = -air_data['T_C']
    air_data['T_F'] = convert_Celsius_to_Fahrenheit(air_data['T_C'])
    air_data['P_Pa'] = ((rawData[5] << 24) + (rawData[4] <<
                        16) + (rawData[3] << 8) + rawData[2])
    air_data['H_pc'] = rawData[6] + (float(rawData[7])/10.0)
    air_data['G_ohm'] = ((rawData[11] << 24) + (rawData[10]
                         << 16) + (rawData[9] << 8) + rawData[8])
    air_data['F_unit'] = const.FAHRENHEIT_SYMBOL
    air_data['C_unit'] = const.CELSIUS_SYMBOL
    if (USE_FAHRENHEIT):
        air_data['T'] = air_data['T_F']
        air_data['T_unit'] = air_data['F_unit']
    else:
        air_data['T'] = air_data['T_C']
        air_data['T_unit'] = air_data['C_unit']
    return air_data


def extractAirQualityData(rawData):
    if len(rawData) != const.AIR_QUALITY_DATA_BYTES:
        raise ValueError('Incorrect number of Air Quality Data bytes')
    air_quality_data = {'AQI': 0, 'CO2e': 0, 'bVOC': 0, 'AQI_accuracy': 0}
    air_quality_data['AQI'] = (rawData[0] + (rawData[1] << 8)
                               + (float(rawData[2])/10.0))
    air_quality_data['CO2e'] = (rawData[3] + (rawData[4] << 8)
                                + (float(rawData[5])/10.0))
    air_quality_data['bVOC'] = (rawData[6] + (rawData[7] << 8)
                                + (float(rawData[8])/100.0))
    air_quality_data['AQI_accuracy'] = rawData[9]
    return air_quality_data


def extractLightData(rawData):
    if len(rawData) != const.LIGHT_DATA_BYTES:
        raise ValueError('Incorrect number of Light Data bytes')
    light_data = {'illum_lux': 0, 'white': 0}
    light_data['illum_lux'] = (rawData[0] + (rawData[1] << 8)
                               + (float(rawData[2])/100.0))
    light_data['white'] = rawData[3] + (rawData[4] << 8)
    return light_data


def extractSoundData(rawData):
    if len(rawData) != const.SOUND_DATA_BYTES:
        raise ValueError('Incorrect number of Sound Data bytes')
    sound_data = {'SPL_dBA': 0,
                  'SPL_bands_dB': [0]*const.SOUND_FREQ_BANDS,
                  'peak_amp_mPa': 0, 'stable': 0}
    sound_data['SPL_dBA'] = rawData[0] + (float(rawData[1])/10.0)
    offset = 2
    for band in range(const.SOUND_FREQ_BANDS):
        sound_data['SPL_bands_dB'][band] = (rawData[offset]
            + (float(rawData[offset + const.SOUND_FREQ_BANDS])/10.0))
        offset += 1
    offset += const.SOUND_FREQ_BANDS
    sound_data['peak_amp_mPa'] = (rawData[offset] + (rawData[offset + 1] << 8)
                                  + (float(rawData[offset + 2])/100.0))
    sound_data['stable'] = rawData[offset + 3]
    return sound_data


def extractParticleData(rawData, particleSensor):
    particle_data = {'duty_cycle_pc': 0,
                     'concentration': 0, 'conc_unit': "", 'valid': False}
    if particleSensor == const.PARTICLE_SENSOR_OFF:
        return particle_data
    if len(rawData) != const.PARTICLE_DATA_BYTES:
        raise ValueError('Incorrect number of Particle Data bytes')
    particle_data['duty_cycle_pc'] = rawData[0] + (float(rawData[1])/100.0)
    particle_data['concentration'] = (rawData[2] + (rawData[3] << 8)
                                      + (float(rawData[4])/100.0))
    if rawData[5] > 0:
        particle_data['valid'] = True
    if particleSensor == const.PARTICLE_SENSOR_PPD42:
        particle_data['conc_unit'] = "ppL"
    elif particleSensor == const.PARTICLE_SENSOR_SDS011:
        particle_data['conc_unit'] = const.SDS011_CONC_SYMBOL
    return particle_data

#############################################################################

# "get_*_data" are functions to read data over I2C and then return 
# Python dictionaries containing the environmental data values.

def get_air_data(I2C_bus):
    raw_data = I2C_bus.read_i2c_block_data(
        i2c_7bit_address, const.AIR_DATA_READ, const.AIR_DATA_BYTES)
    return extractAirData(raw_data)


def get_air_quality_data(I2C_bus):
    raw_data = I2C_bus.read_i2c_block_data(
        i2c_7bit_address, const.AIR_QUALITY_DATA_READ,
        const.AIR_QUALITY_DATA_BYTES)
    return extractAirQualityData(raw_data)


def get_light_data(I2C_bus):
    raw_data = I2C_bus.read_i2c_block_data(
        i2c_7bit_address, const.LIGHT_DATA_READ, const.LIGHT_DATA_BYTES)
    return extractLightData(raw_data)


def get_sound_data(I2C_bus):
    raw_data = I2C_bus.read_i2c_block_data(
        i2c_7bit_address, const.SOUND_DATA_READ, const.SOUND_DATA_BYTES)
    return extractSoundData(raw_data)


def get_particle_data(I2C_bus, particleSensor):
    raw_data = I2C_bus.read_i2c_block_data(
        i2c_7bit_address, const.PARTICLE_DATA_READ, const.PARTICLE_DATA_BYTES)
    return extractParticleData(raw_data, particleSensor)

#############################################################################

def convert_Celsius_to_Fahrenheit(temperature_C):
    """Function to convert Celsius temperature to Fahrenheit."""
    return ((temperature_C*1.8) + 32.0)

#############################################################################

def interpret_AQI_accuracy(AQI_accuracy_code):
    """Provide a readable interpretation of the air quality accuracy code."""
    if (AQI_accuracy_code == 1):
        return "Low accuracy, self-calibration ongoing"
    elif (AQI_accuracy_code == 2):
        return "Medium accuracy, self-calibration ongoing"
    elif (AQI_accuracy_code == 3):
        return "High accuracy"
    else:
        return "Not yet valid, self-calibration incomplete"


def interpret_AQI_value(AQI):
    """Provide a readable interpretation of the AQI (air quality index)."""
    if (AQI < 50):
        return "Good"
    elif (AQI < 100):
        return "Acceptable"
    elif (AQI < 150):
        return "Substandard"
    elif (AQI < 200):
        return "Poor"
    elif (AQI < 300):
        return "Bad"
    else:
        return "Very bad"

#############################################################################

# "write*Data": functions to write data to a text file, or to the screen
# (if the Python script is run in a terminal).
#
# Each function takes 3 arguments:
#
#   textFileObject = An open text file object (to write to file), or
#                    None (to write to screen)

#   XXXData = The data dictionary previously returned by the
#             function named extractXXXData(), where
#             XXX = Air, Sound, Light, AirQuality, Particle

#   writeAsColumns = Boolean; if False, values are written one per line,
#                    labeled with name and measurement unit.
#                    If True, values are written in columns (suitable for
#                    spreadsheets), without labels or units.


def writeAirData(textFileObject, air_data, writeAsColumns):
    """Air data column order is:
    Temperature/C, Pressure/Pa, Humidity/%RH,
    Gas sensor resistance/ohm
    """
    if textFileObject is None:
        textFileObject = sys.stdout
    if writeAsColumns:
        textFileObject.write(f"{air_data['T']:.1f} "
                             f"{air_data['P_Pa']} "
                             f"{air_data['H_pc']:.1f} "
                             f"{air_data['G_ohm']} ")
    else:
        textFileObject.write(f"Temperature = {air_data['T']:.1f} "
                             f"{air_data['T_unit']}\n")
        textFileObject.write(f"Pressure = {air_data['P_Pa']} Pa\n")
        textFileObject.write(f"Humidity = {air_data['H_pc']:.1f} %\n")
        textFileObject.write(f"Gas Sensor Resistance = {air_data['G_ohm']} "
                             + const.OHM_SYMBOL + "\n")


def writeAirQualityData(textFileObject, air_quality_data, writeAsColumns):
    """Air quality data column order is:
    Air Quality Index, Estimated CO2/ppm,
    Equivalent breath VOC/ppm, Accuracy
    """
    if textFileObject is None:
        textFileObject = sys.stdout
    if writeAsColumns:
        textFileObject.write(f"{air_quality_data['AQI']:.1f} "
                             f"{air_quality_data['CO2e']:.1f} "
                             f"{air_quality_data['bVOC']:.2f} "
                             f"{air_quality_data['AQI_accuracy']} ")
    else:
        if air_quality_data['AQI_accuracy'] > 0:
            textFileObject.write(
                f"Air Quality Index = {air_quality_data['AQI']:.1f}"
                f" ({interpret_AQI_value(air_quality_data['AQI'])})\n")
            textFileObject.write(f"Estimated CO{const.SUBSCRIPT_2}"
                                 f" = {air_quality_data['CO2e']:.1f} ppm\n")
            textFileObject.write("Equivalent Breath VOC = "
                                 f"{air_quality_data['bVOC']:.2f} ppm\n")
        textFileObject.write("Air Quality Accuracy: " + interpret_AQI_accuracy(
            air_quality_data['AQI_accuracy']) + "\n")


def writeLightData(textFileObject, light_data, writeAsColumns):
    """Light data column order is:
    Illuminance/lux, white light level
    """
    if textFileObject is None:
        textFileObject = sys.stdout
    if writeAsColumns:
        textFileObject.write(f"{light_data['illum_lux']:.2f} "
                             f"{light_data['white']} ")
    else:
        textFileObject.write(
            f"Illuminance = {light_data['illum_lux']:.2f} lux\n")
        textFileObject.write(f"White Light Level = {light_data['white']}\n")


def writeSoundData(textFileObject, sound_data, writeAsColumns):
    """Sound data column order is:
    Sound pressure level/dBA,
    Sound pressure level for frequency bands 1 to 6 (six columns),
    Peak sound amplitude/mPa, stability
    """
    if textFileObject is None:
        textFileObject = sys.stdout
    if writeAsColumns:
        textFileObject.write(f"{sound_data['SPL_dBA']:.1f} ")
        for band in range(const.SOUND_FREQ_BANDS):
            textFileObject.write(f"{sound_data['SPL_bands_dB'][band]:.1f} ")
        textFileObject.write(f"{sound_data['peak_amp_mPa']:.2f} "
                             f"{sound_data['stable']} ")
    else:
        textFileObject.write("A-weighted Sound Pressure Level = "
                             f"{sound_data['SPL_dBA']:.1f} dBA\n")
        for band in range(const.SOUND_FREQ_BANDS):
            textFileObject.write(f"Frequency Band {band + 1} "
                                 f"({const.sound_band_mids_Hz[band]} Hz) "
                    f"SPL = {sound_data['SPL_bands_dB'][band]:.1f} dB\n")
        textFileObject.write("Peak Sound Amplitude = "
                             f"{sound_data['peak_amp_mPa']:.2f} mPa\n")


def writeParticleData(textFileObject, particle_data, writeAsColumns):
    """Particle data column order is:
    Sensor duty cycle/%, particle concentration
    """
    if textFileObject is None:
        textFileObject = sys.stdout
    if writeAsColumns:
        textFileObject.write(f"{particle_data['duty_cycle_pc']:.2f} "
                             f"{particle_data['concentration']:.2f} ")
        if particle_data['valid']:
            textFileObject.write("1 ")
        else:
            textFileObject.write("0 ")
    else:
        textFileObject.write("Particle Sensor Duty Cycle = "
                             f"{particle_data['duty_cycle_pc']:.2f} %\n")
        textFileObject.write("Particle Concentration = "
                             f"{particle_data['concentration']:.2f} "
                             f"{particle_data['conc_unit']}\n")
        if particle_data['valid'] == 0:
            textFileObject.write("Particle data valid: No (Initializing)\n")
        else:
            textFileObject.write("Particle data valid: Yes\n")

#############################################################################


def startNewDataFile(dataFileDirectory):
    """Open an output text data file.

    Append to an existing file, else create a new one.
    """
    filename = os.path.join(
        dataFileDirectory,
        datetime.now().strftime('data_%Y-%m-%d_%H-%M-%S.txt'))
    print("Logging data to file " + filename)
    return open(filename, 'a')


def setSoundInterruptThreshold(I2C_bus, sound_thres_mPa):
    """Set the threshold for triggering a sound interrupt.

    sound_thres_mPa = peak sound amplitude threshold in
                      milliPascals; any 16-bit integer is allowed.
    """
    data_to_send = [(sound_thres_mPa & 0x00FF), (sound_thres_mPa >> 8)]
    I2C_bus.write_i2c_block_data(
        i2c_7bit_address, const.SOUND_INTERRUPT_THRESHOLD_REG, data_to_send)


def setLightInterruptThreshold(I2C_bus, light_thres_lux_i,
                               light_thres_lux_f2dp):
    """Set the threshold for triggering a light interrupt.

    The threshold value in lux units can be fractional and is formed as:
        threshold = light_thres_lux_i + (light_thres_lux_f2dp / 100)
    Threshold values exceeding MAX_LUX_VALUE will be limited to MAX_LUX_VALUE.
    """
    data_to_send = [(light_thres_lux_i & 0x00FF),
                    (light_thres_lux_i >> 8), light_thres_lux_f2dp]
    I2C_bus.write_i2c_block_data(
        i2c_7bit_address, const.LIGHT_INTERRUPT_THRESHOLD_REG, data_to_send)
