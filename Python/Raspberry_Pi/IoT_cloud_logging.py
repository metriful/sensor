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
from sensor_package.sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# How often to read and log data (every 3, 100, 300 seconds)
# Note: due to data rate limits on free cloud services, this should 
# be set to 100 or 300 seconds, not 3 seconds.
cycle_period = CYCLE_PERIOD_100_S

# IoT cloud settings.

# This example uses the free IoT cloud hosting services provided 
# by Tago.io or Thingspeak.com
# Other free cloud providers are available.
# An account must have been set up with the relevant cloud provider and 
# an internet connection to the Pi must exist. See the accompanying 
# readme and User Guide for more information.

# Choose which provider to use
use_Tago_cloud = True   # set this False to use the Thingspeak cloud

# The chosen account's key/token must be inserted below.  
if (use_Tago_cloud):
  # settings for Tago.io cloud
  TAGO_DEVICE_TOKEN_STRING = "PASTE YOUR TOKEN HERE WITHIN QUOTES"
else:
  # settings for ThingSpeak.com cloud
  THINGSPEAK_API_KEY_STRING = "PASTE YOUR API KEY HERE WITHIN QUOTES"

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Apply the chosen settings to the MS430
I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [PARTICLE_SENSOR])
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
  # Choose output temperature unit (C or F) in sensor_functions.py
  air_data = get_air_data(I2C_bus)
  
  # Air quality data
  # The initial self-calibration of the air quality data may take several
  # minutes to complete. During this time the accuracy parameter is zero 
  # and the data values are not valid.
  air_quality_data = get_air_quality_data(I2C_bus)
    
  # Light data
  light_data = get_light_data(I2C_bus)

  # Sound data
  sound_data = get_sound_data(I2C_bus)
    
  # Particle data
  # This requires the connection of a particulate sensor (zero/invalid 
  # values will be obtained if this sensor is not present).
  # Specify your sensor model (PPD42 or SDS011) in sensor_functions.py
  # Also note that, due to the low pass filtering used, the 
  # particle data become valid after an initial initialization 
  # period of approximately one minute.
  particle_data = get_particle_data(I2C_bus, PARTICLE_SENSOR)
    
  # Assemble the data into the required format, then send it to the cloud
  # as an HTTP POST request.
  
  # For both example cloud providers, the following quantities will be sent:
  # 1 Temperature (measurement unit is selected in sensor_functions.py)
  # 2 Pressure/Pa
  # 3 Humidity/%
  # 4 Air quality index
  # 5 bVOC/ppm
  # 6 SPL/dBA
  # 7 Illuminance/lux
  # 8 Particle concentration
  
  # Additionally, for Tago, the following are sent:
  # 9  Air Quality Assessment summary (Good, Bad, etc.) 
  # 10 Peak sound amplitude / mPa 
  
  try:
    if use_Tago_cloud:
      payload = [0]*10;
      payload[0] = {"variable":"temperature","value":"{:.1f}".format(air_data['T'])}
      payload[1] = {"variable":"pressure","value":air_data['P_Pa']}
      payload[2] = {"variable":"humidity","value":"{:.1f}".format(air_data['H_pc'])}
      payload[3] = {"variable":"aqi","value":"{:.1f}".format(air_quality_data['AQI'])}
      payload[4] = {"variable":"aqi_string","value":interpret_AQI_value(air_quality_data['AQI'])}
      payload[5] = {"variable":"bvoc","value":"{:.2f}".format(air_quality_data['bVOC'])}
      payload[6] = {"variable":"spl","value":"{:.1f}".format(sound_data['SPL_dBA'])}
      payload[7] = {"variable":"peak_amp","value":"{:.2f}".format(sound_data['peak_amp_mPa'])}
      payload[8] = {"variable":"illuminance","value":"{:.2f}".format(light_data['illum_lux'])}
      payload[9] = {"variable":"particulates","value":"{:.2f}".format(particle_data['concentration'])}
      requests.post(tago_url, json=payload, headers=tago_header, timeout=2)
    else:
      # Use ThingSpeak.com cloud
      payload = "api_key=" + THINGSPEAK_API_KEY_STRING 
      payload += "&field1=" + "{:.1f}".format(air_data['T'])
      payload += "&field2=" + str(air_data['P_Pa'])
      payload += "&field3=" + "{:.1f}".format(air_data['H_pc'])
      payload += "&field4=" + "{:.1f}".format(air_quality_data['AQI'])
      payload += "&field5=" + "{:.2f}".format(air_quality_data['bVOC'])
      payload += "&field6=" + "{:.1f}".format(sound_data['SPL_dBA'])
      payload += "&field7=" + "{:.2f}".format(light_data['illum_lux'])
      payload += "&field8=" + "{:.2f}".format(particle_data['concentration'])
      requests.post(thingspeak_url, data=payload, headers=thingspeak_header, timeout=2)
      
  except Exception as e:
    # An error has occurred, likely due to a lost internet connection, 
    # and the post has failed.
    # The program will retry with the next data release and will succeed 
    # if the internet reconnects.
    print("HTTP POST failed with the following error:")
    print(repr(e))
    print("The program will continue and retry on the next data output.")

