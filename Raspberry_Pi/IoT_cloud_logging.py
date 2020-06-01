#  IoT_cloud_logging.py
   
#  Example IoT data logging code for the Sense board. 
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Environmental data values are measured and logged to an internet 
#  cloud account every 100 seconds. The example gives the choice of 
#  using either the Tago.io or Thingspeak.com cloud - both of these 
#  offer a free account for low data rates (compatible with this demo). 

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit https://github.com/metriful/sense

import requests
from Sense_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# How often to read and log data (every 3, 100, 300 seconds)
# Note that due to data rate limits on free cloud services, this should 
# be set to 100 or 300 seconds, not 3 seconds.
cycle_period = CYCLE_PERIOD_100_S

# Whether to read the particle data (set False if no PPD42 particle 
# sensor is connected, to avoid seeing spurious data).
get_particle_data = True

# IoT cloud settings.
# This example demonstrates use of the free IoT cloud hosting 
# services provided by Tago.io or Thingspeak.com
# Other free cloud providers are available.
# An account must have been set up with the relevant cloud provider and 
# an internet connection must exist. See the accompanying readme and 
# User Guide for more information.

# Choose which provider to use
use_Tago_cloud = True 
# To use the ThingSpeak cloud, set: use_Tago_cloud=False

# The chosen account's key/token must be put into the relevant settings information below.  
if (use_Tago_cloud):
  # settings for Tago.io cloud
  TAGO_DEVICE_TOKEN_STRING = "PASTE YOUR TOKEN HERE WITHIN QUOTES"
else:
  # settings for ThingSpeak.com cloud
  THINGSPEAK_API_KEY_STRING = "PASTE YOUR API KEY HERE WITHIN QUOTES"

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SenseHardwareSetup()

# Apply the chosen settings to the Sense board
if (get_particle_data):
  I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_ENABLE_REG, [ENABLED])
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

# Tell the Pi to monitor READY for a falling edge event (high-to-low voltage change)
GPIO.add_event_detect(READY_pin, GPIO.FALLING) 

# Tell Sense to enter cycle mode
I2C_bus.write_byte(i2c_7bit_address, CYCLE_MODE_CMD)

while (True):

  # Wait for the next new data release, indicated by a falling edge on READY
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)
  
  # Now read all data from Sense

  # Air data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_DATA_READ, AIR_DATA_BYTES)
  air_data = extractAirData(raw_data)
  
  # Air quality data
  # Note that the initial self-calibration of the air quality data 
  # takes a few minutes to complete. During this time the accuracy 
  # parameter is zero and the data values do not change.
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, AIR_QUALITY_DATA_READ, AIR_QUALITY_DATA_BYTES)
  air_quality_data = extractAirQualityData(raw_data)
    
  # Light data
  raw_data = I2C_bus.read_i2c_block_data(i2c_7bit_address, LIGHT_DATA_READ, LIGHT_DATA_BYTES)
  light_data = extractLightData(raw_data)
  
  # Sound data
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
    
  # Assemble the data into the required format, then send it to the cloud
  # as an HTTP POST request. Limit the number of decimal places for the
  # variables which are not integers.
  try:
    if use_Tago_cloud:
      # Use Tago.io cloud
      payload = [0]*10;
      payload[0] = {"variable":"temperature","value":"{:.1f}".format(air_data['T_C'])}
      payload[1] = {"variable":"pressure","value":air_data['P_Pa']}
      payload[2] = {"variable":"humidity","value":"{:.1f}".format(air_data['H_pc'])}
      payload[3] = {"variable":"aqi","value":"{:.1f}".format(air_quality_data['AQI'])}
      payload[4] = {"variable":"aqi_string","value":interpret_AQI_value(air_quality_data['AQI'])}
      payload[5] = {"variable":"bvoc","value":"{:.2f}".format(air_quality_data['bVOC'])}
      payload[6] = {"variable":"spl","value":"{:.1f}".format(sound_data['SPL_dBA'])}
      payload[7] = {"variable":"peak_amp","value":"{:.2f}".format(sound_data['peak_amp_mPa'])}
      payload[8] = {"variable":"illuminance","value":"{:.2f}".format(light_data['illum_lux'])}
      payload[9] = {"variable":"particulates","value":particle_data['conc_ppL']}
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
      payload += "&field8=" + str(particle_data['conc_ppL'])
      requests.post(thingspeak_url, data=payload, headers=thingspeak_header, timeout=2)
      
  except:
    # An error has occurred, likely due to a lost internet connection, 
    # and the post has failed.
    # The program will retry with the next data release and will succeed 
    # if the internet reconnects.
    print("HTTP POST failed.")



