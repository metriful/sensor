#  Home_Assistant.py

#  Example code for sending environment data from the Metriful MS430 to
#  an installation of Home Assistant (www.home-assistant.io) on your 
#  home network.
#  This example is designed to run with Python 3 on a Raspberry Pi.

#  Data are sent at regular intervals over your local network to Home 
#  Assistant and can be viewed on the dashboard and used to control 
#  home automation tasks. More setup information is provided in the 
#  Readme and User Guide.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

import requests
import logging
from sensor_package.sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# How often to read and report the data (every 3, 100 or 300 seconds)
cycle_period = CYCLE_PERIOD_100_S

# Home Assistant settings

# You must have already installed Home Assistant on a computer on your 
# network. Go to www.home-assistant.io for help on this.

# Choose a unique name for this MS430 sensor board so you can identify it.
# Variables in HA will have names like: SENSOR_NAME.temperature, etc.
SENSOR_NAME = "ms430"

# Specify the URL of the Home Assistant instance. 
HOME_ASSISTANT_URL = "http://192.168.43.144:8123"
# Set to false for unverified SSL connections
HOME_ASSISTANT_SSL_VERIFY = True

# Security access token: the Readme and User Guide explain how to get this
LONG_LIVED_ACCESS_TOKEN = ""

#Valid logging levels defined at https://docs.python.org/3/library/logging.html#levels
LOG_LEVEL="ERROR"

# END OF USER-EDITABLE SETTINGS
#########################################################

logger = logging.getLogger()
logger.setLevel(LOG_LEVEL.upper())
log_format = logging.Formatter(
    '%(asctime)s.%(msecs)03d %(levelname)s %(module)s - %(funcName)s: %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
consolelog = logging.StreamHandler()
consolelog.setFormatter(log_format)
logger.addHandler(consolelog)

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Apply the settings to the MS430
I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [PARTICLE_SENSOR])
I2C_bus.write_i2c_block_data(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, [cycle_period])

#########################################################

logging.info("Reporting data to Home Assistant. Press ctrl-c to exit.")

# Enter cycle mode
I2C_bus.write_byte(i2c_7bit_address, CYCLE_MODE_CMD)

while (True):
  # Wait for the next new data release, indicated by a falling edge on READY
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)
  
  try:
    logging.info("Starting uploading cycle")
    # Now read all data from the MS430
    air_data = get_air_data(I2C_bus)
    air_quality_data = get_air_quality_data(I2C_bus)
    light_data = get_light_data(I2C_bus)
    sound_data = get_sound_data(I2C_bus)
    particle_data = get_particle_data(I2C_bus, PARTICLE_SENSOR)
  except Exception as e:
    logging.error("Error getting data from sensor...")
    logging.debug(repr(e))
    logging.info("The program will continue and retry on the next data output.")
  
  # Specify information needed by Home Assistant.
  # Icons are chosen from https://cdn.materialdesignicons.com/5.3.45/    
  # (remove the "mdi-" part from the icon name).
  pressure = dict(name='Pressure', data=air_data['P_Pa'], unit='Pa', icon='weather-cloudy', decimals=0)
  humidity = dict(name='Humidity', data=air_data['H_pc'], unit='%', icon='water-percent', decimals=1)
  temperature = dict(name='Temperature', data=air_data['T'], unit=air_data['T_unit'], 
                     icon='thermometer', decimals=1)
  illuminance = dict(name='Illuminance', data=light_data['illum_lux'], unit='lx', 
                     icon='white-balance-sunny', decimals=2)
  sound_level = dict(name='Sound level', data=sound_data['SPL_dBA'], unit='dBA', 
                     icon='microphone', decimals=1)
  sound_peak = dict(name='Sound peak', data=sound_data['peak_amp_mPa'], unit='mPa', 
                     icon='waveform', decimals=2)
  AQI = dict(name='Air Quality Index', data=air_quality_data['AQI'], unit=' ', 
             icon='thought-bubble-outline', decimals=1)
  AQI_interpret = dict(name='Air quality assessment', data=interpret_AQI_value(air_quality_data['AQI']),
                       unit='', icon='flower-tulip', decimals=0)
  particle = dict(name='Particle concentration', data=particle_data['concentration'], 
                  unit=particle_data['conc_unit'], icon='chart-bubble', decimals=2)

  # Send data to Home Assistant using HTTP POST requests
  variables = [pressure, humidity, temperature, illuminance, sound_level, sound_peak, AQI, AQI_interpret]
  if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF):
    variables.append(particle)
  try:
    for v in variables:
      url = (HOME_ASSISTANT_URL + "/api/states/" + 
            "sensor." + SENSOR_NAME + "_" + v['name'].replace(' ','_').lower())
      head = {"Content-type": "application/json","Authorization": "Bearer " + LONG_LIVED_ACCESS_TOKEN}
      try:
        valueStr = "{:.{dps}f}".format(v['data'], dps=v['decimals'])
      except:
        valueStr = v['data']
      payload = {"state":valueStr, "attributes":{"unique_id":SENSOR_NAME, "unit_of_measurement":v['unit'],
                 "friendly_name":v['name'], "icon":"mdi:" + v['icon']}}
      logging.debug("Info uploaded: %s" % payload)
      request = requests.post(url, json=payload, headers=head, timeout=2, verify=HOME_ASSISTANT_SSL_VERIFY)
  except Exception as e:
    # An error has occurred, likely due to a lost network connection, 
    # and the post has failed.
    # The program will retry with the next data release and will succeed 
    # if the network reconnects.
    logging.error("HTTP(S) POST failed with the following error:")
    logging.debug(repr(e))
    logging.info("The program will continue and retry on the next data output.")
  logging.info("Waiting until next cycle")

