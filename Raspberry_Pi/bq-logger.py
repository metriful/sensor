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
import io
from sensor_functions import *
from configparser import ConfigParser
from pathlib import Path
from google.cloud import bigquery
from google.oauth2 import service_account
import datetime

#########################################################
# config

config = ConfigParser()
config.read('{}/.metriful'.format(str(Path.home())))

# Read the Google service account credentials
key_path = '{}/.metriful-service-account.json'.format(str(Path.home()))

credentials = service_account.Credentials.from_service_account_file(
    key_path, scopes=["https://www.googleapis.com/auth/cloud-platform"],
)

# Set table_id to the ID of the table to create.
bq_dataset_id = config.get('main', 'bq_dataset_id')
bq_table_id = config.get('main', 'bq_table_id')
bq_table_id_combined = '{}.{}.{}'.format(credentials.project_id, bq_dataset_id, bq_table_id)

location_id = config.get('main', 'location_id')

# Sensor variables
cycle_period = globals()[config.get('main', 'cycle_period')]
particleSensor = globals()[config.get('main', 'particleSensor')]
add_temperature_f = config.getboolean('main', 'add_temperature_f')
log_to_syslog = config.getboolean('main', 'log_to_syslog')

#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Apply the chosen settings to the MS430
if (particleSensor != PARTICLE_SENSOR_OFF):
  I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [particleSensor])
I2C_bus.write_i2c_block_data(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, [cycle_period])

#########################################################

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
    data = {
      "temperature":"{:.1f}".format(air_data['T_C']),
      "pressure":air_data['P_Pa'],
      "humidity":"{:.1f}".format(air_data['H_pc']),
      "aqi":"{:.1f}".format(air_quality_data['AQI']),
      "aqi_string":interpret_AQI_value(air_quality_data['AQI']),
      "bvoc":"{:.2f}".format(air_quality_data['bVOC']),
      "spl":"{:.1f}".format(sound_data['SPL_dBA']),
      "peak_amp":"{:.2f}".format(sound_data['peak_amp_mPa']),
      "illuminance":"{:.2f}".format(light_data['illum_lux']),
      "particulates":"{:.2f}".format(particle_data['concentration']),
      "timestamp":datetime.datetime.now().isoformat(),
      "location":location_id,
      "temperature_f":"{:.1f}".format((air_data['T_C']*9/5)+32)
    }

    data_json = json.dumps(data)

    if log_to_syslog:
      syslog.syslog(data_json)

    # Initialize the client
    # ---------------------------
    client = bigquery.Client(credentials=credentials, project=credentials.project_id,)
    dataset  = client.dataset(bq_dataset_id)
    table = dataset.table(bq_table_id)

    job_config = bigquery.LoadJobConfig()
    job_config.source_format = bigquery.SourceFormat.NEWLINE_DELIMITED_JSON
    job_config.autodetect = True

    # Wrap the data into a file-like object and pass it to load_table_from_file instead of load_table_from_json
    # ---------------------------
    # https://googleapis.dev/python/bigquery/latest/generated/google.cloud.bigquery.client.Client.html#google.cloud.bigquery.client.Client.load_table_from_json
    data_as_file = io.StringIO(data_json)
    job = client.load_table_from_file(data_as_file, table, job_config=job_config)

    if log_to_syslog:
      syslog.syslog(job.result())

  except Exception as e:
    # The script will retry with the next data set
    if log_to_syslog:
      syslog.syslog("Load data failed.\nERROR: {}".format(e))

