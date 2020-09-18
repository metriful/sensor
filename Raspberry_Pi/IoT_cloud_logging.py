#  IoT_cloud_logging.py
   
#  Example IoT data logging code for the Metriful MS430. 
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Environmental data values are measured and logged to an internet 
#  cloud account every 100 seconds. The example gives the choice of 
#  using either the Tago.io or Thingspeak.com cloud - both of these 
#  offer a free account for low data rates. 

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

import requests
import syslog
import json
from sensor_functions import *
from configparser import ConfigParser
from pathlib import Path

#########################################################
# config

config = ConfigParser()
config.read('{}/.metriful'.format(str(Path.home())))

cycle_period = globals()[config.get('main', 'cycle_period')]
particleSensor = globals()[config.get('main', 'particleSensor')]
use_Tago_cloud = config.getboolean('main', 'use_Tago_cloud')
add_temperature_f = config.getboolean('main', 'add_temperature_f')
log_to_syslog = config.getboolean('main', 'log_to_syslog')
if (use_Tago_cloud):
  TAGO_DEVICE_TOKEN_STRING = config.get('main', 'TAGO_DEVICE_TOKEN_STRING')
else:
  THINGSPEAK_API_KEY_STRING = config.get('main', 'THINGSPEAK_API_KEY_STRING')

#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Apply the chosen settings to the MS430
if (particleSensor != PARTICLE_SENSOR_OFF):
  I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [particleSensor])
I2C_bus.write_i2c_block_data(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, [cycle_period])

#########################################################

# Full cloud settings for HTTP logging
if (use_Tago_cloud):
  # settings for Tago.io cloud
  tago_url = "http://api.tago.io/data"
  tago_header = {"Content-type": "application/json","Device-Token":TAGO_DEVICE_TOKEN_STRING}
else:
  # settings for ThingSpeak.com cloud
  thingspeak_url = "http://api.thingspeak.com/update"
  thingspeak_header = {"Content-type": "application/x-www-form-urlencoded"} 

print("Logging data. Press ctrl-c to exit.")

# Enter cycle mode
I2C_bus.write_byte(i2c_7bit_address, CYCLE_MODE_CMD)

while (True):

  # Wait for the next new data release, indicated by a falling edge on READY
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)
  
  # Now read all data from the MS430

  # Air data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_DATA_READ, AIR_DATA_BYTES)
  air_data = extractAirData(raw_data)
  
  # Air quality data
  # The initial self-calibration of the air quality data may take several
  # minutes to complete. During this time the accuracy parameter is zero 
  # and the data values are not valid.
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_QUALITY_DATA_READ, AIR_QUALITY_DATA_BYTES)
  air_quality_data = extractAirQualityData(raw_data)
    
  # Light data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, LIGHT_DATA_READ, LIGHT_DATA_BYTES)
  light_data = extractLightData(raw_data)
  
  # Sound data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, SOUND_DATA_READ, SOUND_DATA_BYTES)
  sound_data = extractSoundData(raw_data)
    
  # Particle data
  # This requires the connection of a particulate sensor (invalid 
  # values will be obtained if this sensor is not present).
  # Also note that, due to the low pass filtering used, the 
  # particle data become valid after an initial initialization 
  # period of approximately one minute.
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, PARTICLE_DATA_READ, PARTICLE_DATA_BYTES)
  particle_data = extractParticleData(raw_data, particleSensor)
    
  # Assemble the data into the required format, then send it to the cloud
  # as an HTTP POST request.
  
  # For both example cloud providers, the following quantities will be sent:
  # 1 Temperature/C
  # 2 Pressure/Pa
  # 3 Humidity/%
  # 4 Air quality index
  # 5 bVOC/ppm
  # 6 SPL/dBA
  # 7 Illuminance/lux
  # 8 Particle concentration
  
  # Additionally, for Tago, the following is sent:
  # 9  Air Quality Assessment summary (Good, Bad, etc.) 
  # 10 Peak sound amplitude / mPa 
  # 11 Temperature/F
  
  try:
    if use_Tago_cloud:
      payload = [0]*11 if add_temperature_f else [0]*10
      payload[0] = {"variable":"temperature","value":"{:.1f}".format(air_data['T_C'])}
      payload[1] = {"variable":"pressure","value":air_data['P_Pa']}
      payload[2] = {"variable":"humidity","value":"{:.1f}".format(air_data['H_pc'])}
      payload[3] = {"variable":"aqi","value":"{:.1f}".format(air_quality_data['AQI'])}
      payload[4] = {"variable":"aqi_string","value":interpret_AQI_value(air_quality_data['AQI'])}
      payload[5] = {"variable":"bvoc","value":"{:.2f}".format(air_quality_data['bVOC'])}
      payload[6] = {"variable":"spl","value":"{:.1f}".format(sound_data['SPL_dBA'])}
      payload[7] = {"variable":"peak_amp","value":"{:.2f}".format(sound_data['peak_amp_mPa'])}
      payload[8] = {"variable":"illuminance","value":"{:.2f}".format(light_data['illum_lux'])}
      payload[9] = {"variable":"particulates","value":"{:.2f}".format(particle_data['concentration'])}
      if add_temperature_f:
        payload[10] = {"variable":"temperature_f","value":"{:.1f}".format((air_data['T_C']*9/5)+32)}
      if log_to_syslog:
        syslog.syslog(json.dumps(payload))
      requests.post(tago_url, json=payload, headers=tago_header, timeout=2)
    else:
      # Use ThingSpeak.com cloud
      payload = "api_key=" + THINGSPEAK_API_KEY_STRING 
      payload += "&field1=" + "{:.1f}".format(air_data['T_C'])
      payload += "&field2=" + str(air_data['P_Pa'])
      payload += "&field3=" + "{:.1f}".format(air_data['H_pc'])
      payload += "&field4=" + "{:.1f}".format(air_quality_data['AQI'])
      payload += "&field5=" + "{:.2f}".format(air_quality_data['bVOC'])
      payload += "&field6=" + "{:.1f}".format(sound_data['SPL_dBA'])
      payload += "&field7=" + "{:.2f}".format(light_data['illum_lux'])
      payload += "&field8=" + "{:.2f}".format(particle_data['concentration'])
      if log_to_syslog:
        syslog.syslog(json.dumps(payload))
      requests.post(thingspeak_url, data=payload, headers=thingspeak_header, timeout=2)
      
  except Exception as e:
    # An error has occurred, likely due to a lost internet connection, 
    # and the post has failed.
    # The program will retry with the next data release and will succeed 
    # if the internet reconnects.
    print("HTTP POST failed.\nERROR: {}".format(e))



