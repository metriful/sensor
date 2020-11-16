#  IFTTT.py

#  Example code for sending data from the Metriful MS430 to IFTTT.com 
#  This example is designed to run with Python 3 on a Raspberry Pi.

#  Environmental data values are periodically measured and compared with
#  a set of user-defined thresholds. If any values go outside the allowed
#  ranges, an HTTP POST request is sent to IFTTT.com, triggering an alert
#  email to your inbox, with customizable text. 

#  More setup information is provided in the readme and User Guide.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

import requests
from sensor_package.sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# IFTTT.com settings: WEBHOOKS_KEY and IFTTT_EVENT_NAME

# You must set up a free account on IFTTT.com and create a Webhooks 
# applet before using this example. This is explained further in the
# instructions in the GitHub Readme and in the User Guide.

WEBHOOKS_KEY = "PASTE YOUR KEY HERE WITHIN QUOTES"
IFTTT_EVENT_NAME = "PASTE YOUR EVENT NAME HERE WITHIN QUOTES"

# An inactive period follows each alert, during which the same alert 
# will not be generated again - this prevents too many emails/alerts.
# Choose the period as a number of readout cycles (each 5 minutes) 
# e.g. for a 2 hour period, choose inactive_wait_cycles = 24
inactive_wait_cycles = 24;

# Define the details of the variables for monitoring:
humidity = {'name':'humidity',
           'unit':"%",
           'decimal_places':1,
           'high_threshold':60,
           'low_threshold':30,
           'inactive_count':2,
           'high_advice':'Reduce moisture sources.',
           'low_advice':'Start the humidifier.'}

air_quality_index = {'name':'air quality index',
                     'unit':'',
                     'decimal_places':1,
                     'high_threshold':250,
                     'low_threshold':-1,
                     'inactive_count':2,
                     'high_advice':'Improve ventilation.',
                     'low_advice':''}

# This example assumes that Celsius output temperature is selected. Edit 
# these values if Fahrenheit is selected in sensor_functions.py
temperature = {'name':'temperature',
              'unit':CELSIUS_SYMBOL,
              'decimal_places':1,
              'high_threshold':23,
              'low_threshold':18,
              'inactive_count':2,
              'high_advice':'Turn on the fan.',
              'low_advice':'Turn on the heating.'}

# END OF USER-EDITABLE SETTINGS
#########################################################

# Measure the environment data every 300 seconds (5 minutes). This is 
# adequate for long-term monitoring.
cycle_period = CYCLE_PERIOD_300_S

# IFTTT settings:
IFTTT_url = "http://maker.ifttt.com/trigger/" + IFTTT_EVENT_NAME + "/with/key/" + WEBHOOKS_KEY
IFTTT_header = {"Content-type": "application/json"}

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

#########################################################

print("Monitoring data. Press ctrl-c to exit.")

# Enter cycle mode
I2C_bus.write_i2c_block_data(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, [cycle_period])
I2C_bus.write_byte(i2c_7bit_address, CYCLE_MODE_CMD)

while (True):

  # Wait for the next new data release, indicated by a falling edge on READY
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)
  
  # Read the air data and air quality data
  air_data = get_air_data(I2C_bus)
  air_quality_data = get_air_quality_data(I2C_bus)
  temperature['data'] = air_data['T']
  humidity['data'] = air_data['H_pc']
  air_quality_index['data'] = air_quality_data['AQI']
  
  # Check the new values and send an alert to IFTTT if a variable is 
  # outside its allowed range.
  for v in [temperature, humidity, air_quality_index]:
    
    if (v['inactive_count'] > 0):
      # Count down to when the monitoring is active again
      v['inactive_count']-=1
      
    send_alert = False
    if ((v['data'] > v['high_threshold']) and (v['inactive_count'] == 0)):
      # The variable is above the high threshold: send an alert then 
      # ignore this variable for the next inactive_wait_cycles
      v['inactive_count'] = inactive_wait_cycles
      send_alert = True
      threshold_description = 'high.'
      advice = v['high_advice']
    elif ((v['data'] < v['low_threshold']) and (v['inactive_count'] == 0)):
      # The variable is below the low threshold: send an alert then 
      # ignore this variable for the next inactive_wait_cycles
      v['inactive_count'] = inactive_wait_cycles
      send_alert = True
      threshold_description = 'low.'
      advice = v['low_advice']
  
    if send_alert:
      # Send data using an HTTP POST request
      try:
        value1 = "The " + v['name'] + " is too " + threshold_description
        print("Sending new alert to IFTTT: " + value1)
        payload = {"value1":value1,
                   "value2":("The measurement was {:.{dps}f} ".format(v['data'], 
                              dps=v['decimal_places']) + v['unit']),
                   "value3":advice}
        requests.post(IFTTT_url, json=payload, headers=IFTTT_header, timeout=2)
      except Exception as e:
        # An error has occurred, likely due to a lost internet connection, 
        # and the post has failed. The program will continue and new 
        # alerts will succeed if the internet reconnects.
        print("HTTP POST failed with the following error:")
        print(repr(e))
        print("The program will attempt to continue.")

