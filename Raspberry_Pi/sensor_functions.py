#  sensor_functions.py

#  This file defines functions and hardware pins which are used 
#  in the Metriful code examples on Raspberry Pi.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit https://github.com/metriful/sensor

import sys
from time import sleep
import datetime
import RPi.GPIO as GPIO
import smbus
import os
from sensor_constants import *

##########################################################################################

# GPIO (input/output) header pin numbers: these must match the hardware wiring.
# The interrupt pins are not used in all examples.
light_int_pin = 7 # Raspberry Pi pin 7 connects to LIT
sound_int_pin = 8 # Raspberry Pi pin 8 connects to SIT
READY_pin = 11    # Raspberry Pi pin 11 connects to RDY
# In addition to these GPIO pins, the following I2C and power 
# connections must be made: 
# Raspberry Pi pin 3 to SDA
# Raspberry Pi pin 5 to SCL
# Raspberry Pi pin 6 to GND (0 V)
# Raspberry Pi pin 1 to VDD and VPU (3.3 V)
# Metriful pin VIN is not used.
#
# If a PPD42 particle sensor is used, also connect the following:
# Raspberry Pi pin 2 to PPD42 pin 3
# Raspberry Pi pin 9 to PPD42 pin 1
# PPD42 pin 4 to Metriful pin PRT
#
# For further details, see the readme and User Guide

# The I2C address of the Metriful board
i2c_7bit_address = I2C_ADDR_7BIT_SB_OPEN 

##########################################################################################

def SensorHardwareSetup():
  # Set up the Raspberry Pi GPIO
  GPIO.setwarnings(False)
  GPIO.setmode(GPIO.BOARD) 
  GPIO.setup(READY_pin, GPIO.IN)
  GPIO.setup(light_int_pin, GPIO.IN)
  GPIO.setup(sound_int_pin, GPIO.IN)

  # Initialize the I2C communications bus object
  I2C_bus = smbus.SMBus(1) # Port 1 is the default for I2C on Raspberry Pi    

  # Wait for Metriful to finish power-on initialization:
  while (GPIO.input(READY_pin) == 1):
    sleep(0.05)
    
  # Reset Metriful to clear any previous state:
  I2C_bus.write_byte(i2c_7bit_address, RESET_CMD)
  sleep(0.005)
  
  # Wait for reset completion and entry to standby mode
  while (GPIO.input(READY_pin) == 1):
    sleep(0.05)
  
  return (GPIO, I2C_bus)

##########################################################################################

# Functions to convert the raw data bytes (received over I2C)
# into Python dictionaries containing the environmental data values.

def extractAirData(rawData):
  if (len(rawData) != AIR_DATA_BYTES):
    raise Exception('Incorrect number of Air Data bytes')
  airData = {'T_C':0, 'P_Pa':0, 'H_pc':0, 'G_ohm':0}
  airData['T_C'] = ((rawData[0] & TEMPERATURE_VALUE_MASK) + (float(rawData[1])/10.0))
  if ((rawData[0] & TEMPERATURE_SIGN_MASK) != 0):
    # the most-significant bit is set, indicating that the temperature is negative
    airData['T_C'] = -airData['T_C']
  airData['P_Pa'] = ((rawData[5] << 24) + (rawData[4] << 16) + (rawData[3] << 8) + rawData[2])
  airData['H_pc'] = rawData[6] + (float(rawData[7])/10.0)
  airData['G_ohm'] = ((rawData[11] << 24) + (rawData[10] << 16) 
            + (rawData[9] << 8) + rawData[8])
  return airData

def extractAirQualityData(rawData):
  if (len(rawData) != AIR_QUALITY_DATA_BYTES):
    raise Exception('Incorrect number of Air Quality Data bytes')
  airQualityData = {'AQI':0, 'CO2e':0, 'bVOC':0, 'AQI_accuracy':0}
  airQualityData['AQI'] =  rawData[0] + (rawData[1] << 8) + (float(rawData[2])/10.0)
  airQualityData['CO2e'] = rawData[3] + (rawData[4] << 8) + (float(rawData[5])/10.0)
  airQualityData['bVOC'] = rawData[6] + (rawData[7] << 8) + (float(rawData[8])/100.0)
  airQualityData['AQI_accuracy'] = rawData[9]
  return airQualityData
  
def extractLightData(rawData):
  if (len(rawData) != LIGHT_DATA_BYTES):
    raise Exception('Incorrect number of Light Data bytes supplied to function')
  lightData = {'illum_lux':0, 'white':0}
  lightData['illum_lux'] =  rawData[0] + (rawData[1] << 8) + (float(rawData[2])/100.0)
  lightData['white'] = rawData[3] + (rawData[4] << 8)
  return lightData
  
def extractSoundData(rawData):
  if (len(rawData) != SOUND_DATA_BYTES):
    raise Exception('Incorrect number of Sound Data bytes supplied to function')
  soundData = {'SPL_dBA':0, 'SPL_bands_dB':[0]*SOUND_FREQ_BANDS, 'peak_amp_mPa':0, 'stable':0}
  soundData['SPL_dBA'] =  rawData[0] + (float(rawData[1])/10.0)
  j=2
  for i in range(0,SOUND_FREQ_BANDS):
    soundData['SPL_bands_dB'][i] = rawData[j] + (float(rawData[j+SOUND_FREQ_BANDS])/10.0)
    j+=1
  j+=SOUND_FREQ_BANDS
  soundData['peak_amp_mPa'] =  rawData[j] + (rawData[j+1] << 8) + (float(rawData[j+2])/100.0)
  soundData['stable'] = rawData[j+3]
  return soundData

def extractParticleData(rawData):
  if (len(rawData) != PARTICLE_DATA_BYTES):
    raise Exception('Incorrect number of Particle Data bytes supplied to function')
  particleData = {'occupancy_pc':0, 'conc_ppL':0}
  particleData['occupancy_pc'] =  rawData[0] + (float(rawData[1])/100.0)
  particleData['conc_ppL'] =  rawData[2] + (rawData[3] << 8) 
  return particleData

##########################################################################################

# Provide a readable interpretation of the accuracy code for 
# the air quality measurements (applies to all air quality data) 
def interpret_AQI_accuracy(AQI_accuracy_code):
  if (AQI_accuracy_code == 1):
    return "Low Accuracy, Calibration Ongoing";
  elif (AQI_accuracy_code == 2):
    return "Medium Accuracy, Calibration Ongoing";
  elif (AQI_accuracy_code == 3):
    return "High Accuracy";
  else:
    return "Not Valid, Calibration Incomplete";

# Provide a readable interpretation of the AQI (air quality index) 
def interpret_AQI_value(AQI):
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
    return "Very Bad"

##########################################################################################

# Functions to write data to a text file, or to the screen (if the Python script is 
# run in a terminal).
#
# Each function takes 3 arguments:
#
#   textFileObject = An open text file object (to write to file), or None (to write to screen)

#   XXXData = The data dictionary previously returned by the function named 
#             extractXXXData(), where XXX = Air, Sound, Light, AirQuality, Particle

#   writeAsColumns = Boolean; if False, values are written one per line, labeled with name and 
#                    measurement unit. If True, values are written in columns (suitable for 
#                    spreadsheets), without labels or units.

# Air data column order is:
# Temperature/C, Pressure/Pa, Humidity/%RH, Gas sensor resistance/ohm
def writeAirData(textFileObject, airData, writeAsColumns):
  if (textFileObject is None):
    textFileObject = sys.stdout
  if (writeAsColumns):
    textFileObject.write("{:.1f} ".format(airData['T_C']))
    textFileObject.write(str(airData['P_Pa']) + " ")
    textFileObject.write("{:.1f} ".format(airData['H_pc']))
    textFileObject.write(str(airData['G_ohm']) + " ")
  else:
    textFileObject.write("Temperature = {:.1f} C\n".format(airData['T_C']))
    textFileObject.write("Pressure = " + str(airData['P_Pa']) + " Pa\n")
    textFileObject.write("Humidity = {:.1f} %\n".format(airData['H_pc']))
    textFileObject.write("Gas Sensor Resistance = " + str(airData['G_ohm']) + " ohm\n")
  
# Air quality data column order is:
# Air Quality Index, Estimated CO2/ppm, Equivalent breath VOC/ppm, Accuracy
def writeAirQualityData(textFileObject, airQualityData, writeAsColumns):
  if (textFileObject is None):
    textFileObject = sys.stdout
  if (writeAsColumns):
    textFileObject.write("{:.1f} ".format(airQualityData['AQI']))
    textFileObject.write("{:.1f} ".format(airQualityData['CO2e']))
    textFileObject.write("{:.2f} ".format(airQualityData['bVOC']))
    textFileObject.write(str(airQualityData['AQI_accuracy']) + " ")
  else:
    textFileObject.write("Air Quality Index = {:.1f}".format(airQualityData['AQI']) 
          + " (" + interpret_AQI_value(airQualityData['AQI']) + ")\n")
    textFileObject.write("Estimated CO2 = {:.1f} ppm\n".format(airQualityData['CO2e']))
    textFileObject.write("Equivalent Breath VOC = {:.2f} ppm\n".format(airQualityData['bVOC']))
    textFileObject.write("Air Quality Accuracy: " + 
          interpret_AQI_accuracy(airQualityData['AQI_accuracy']) + "\n")
  
# Light data column order is:
# Illuminance/lux, white light level
def writeLightData(textFileObject, lightData, writeAsColumns):
  if (textFileObject is None):
    textFileObject = sys.stdout
  if (writeAsColumns):
    textFileObject.write("{:.2f} ".format(lightData['illum_lux']))
    textFileObject.write(str(lightData['white']) + " ")
  else:
    textFileObject.write("Illuminance = {:.2f} lux\n".format(lightData['illum_lux']))
    textFileObject.write("White Light Level = " + str(lightData['white']) + "\n")

# Sound data column order is:
# Sound pressure level/dBA, Sound pressure level for frequency bands 1 to 6 (six columns), 
# Peak sound amplitude/mPa, stability 
def writeSoundData(textFileObject, soundData, writeAsColumns):
  if (textFileObject is None):
    textFileObject = sys.stdout
  if (writeAsColumns):
    textFileObject.write("{:.1f} ".format(soundData['SPL_dBA']))
    for i in range(0,SOUND_FREQ_BANDS):
      textFileObject.write("{:.1f} ".format(soundData['SPL_bands_dB'][i]))
    textFileObject.write("{:.2f} ".format(soundData['peak_amp_mPa']))
    textFileObject.write(str(soundData['stable']) + " ")
  else:
    textFileObject.write("A-weighted Sound Pressure Level = {:.1f} dBA\n".format(soundData['SPL_dBA']))
    for i in range(0,SOUND_FREQ_BANDS):
      textFileObject.write("Frequency Band " + str(i+1) + " (" + str(sound_band_mids_Hz[i]) 
          + " Hz) SPL = {:.1f} dB\n".format(soundData['SPL_bands_dB'][i]))
    textFileObject.write("Peak Sound Amplitude = {:.2f} mPa\n".format(soundData['peak_amp_mPa']))
    if (soundData['stable'] == 0):
      textFileObject.write("Microphone Initialized: No\n")
    else:
      textFileObject.write("Microphone Initialized: Yes\n")
  
# Particle data column order is:
# Sensor occupancy/%, particle concentration/ppL
def writeParticleData(textFileObject, particleData, writeAsColumns):
  if (textFileObject is None):
    textFileObject = sys.stdout
  if (writeAsColumns):
    textFileObject.write("{:.2f} ".format(particleData['occupancy_pc']))
    textFileObject.write(str(particleData['conc_ppL']) + " ")
  else:
    textFileObject.write("Particle Occupancy = {:.2f} %\n".format(particleData['occupancy_pc']))
    textFileObject.write("Particle Concentration = " + str(particleData['conc_ppL']) + " ppL\n")

##########################################################################################

# Function to open a new output data file, in a specified 
# directory, with a name containing the time and date 
def startNewDataFile(dataFileDirectory):
  filename = os.path.join(dataFileDirectory,datetime.datetime.now().strftime('data_%Y-%m-%d_%H-%M-%S.txt'))
  print("Logging data to file " + filename)
  return open(filename, 'a')
