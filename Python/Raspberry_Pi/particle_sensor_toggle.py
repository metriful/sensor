#  particle_sensor_toggle.py

#  Optional advanced demo. This program shows how to generate an output 
#  control signal from one of the Pi pins, which can be used to turn 
#  the particle sensor on and off. An external transistor circuit is
#  also needed - this will gate the sensor power supply according to 
#  the control signal. Further details are given in the User Guide.
   
#  The program continually measures and displays all environment data
#  in a repeating cycle. The user can view the output in the Serial 
#  Monitor. After reading the data, the particle sensor is powered off 
#  for a chosen number of cycles ("off_cycles"). It is then powered on 
#  and read before being powered off again. Sound data are ignored 
#  while the particle sensor is on, to avoid fan noise.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

from sensor_package.sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# How often to read data; choose only 100 or 300 seconds for this demo
# because the sensor should be on for at least one minute before reading
# its data.
cycle_period = CYCLE_PERIOD_100_S 

# How to print the data: If print_data_as_columns = True,
# data are columns of numbers, useful to copy/paste to a spreadsheet
# application. Otherwise, data are printed with explanatory labels and units.
print_data_as_columns = True

# Particle sensor power control options
off_cycles = 2;  # leave the sensor off for this many cycles between reads
particle_sensor_control_pin = 10; # Pi pin number which outputs the control signal

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

# Set up the particle sensor control, and turn it off initially
GPIO.setup(particle_sensor_control_pin, GPIO.OUT)
GPIO.output(particle_sensor_control_pin, 0)
particle_sensor_is_on = False
particle_sensor_count = 0

# Apply the chosen settings
I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [PARTICLE_SENSOR])
I2C_bus.write_i2c_block_data(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, [cycle_period])

#########################################################

sound_data = extractSoundData([0]*SOUND_DATA_BYTES)
particle_data = extractParticleData([0]*PARTICLE_DATA_BYTES, PARTICLE_SENSOR)

print("Entering cycle mode and waiting for data. Press ctrl-c to exit.")

I2C_bus.write_byte(i2c_7bit_address, CYCLE_MODE_CMD)

while (True):

  # Wait for the next new data release, indicated by a falling edge on READY
  while (not GPIO.event_detected(READY_pin)):
    sleep(0.05)

  # Now read and print all data. The previous loop's particle or
  # sound data will be printed if no reading is done on this loop.

  # Air data
  air_data = get_air_data(I2C_bus)
  writeAirData(None, air_data, print_data_as_columns)

  # Air quality data
  # The initial self-calibration of the air quality data may take several
  # minutes to complete. During this time the accuracy parameter is zero 
  # and the data values are not valid.
  air_quality_data = get_air_quality_data(I2C_bus)
  writeAirQualityData(None, air_quality_data, print_data_as_columns)

  # Light data
  light_data = get_light_data(I2C_bus)
  writeLightData(None, light_data, print_data_as_columns)

  # Sound data - only read new data when particle sensor is off
  if (not particle_sensor_is_on):
    sound_data = get_sound_data(I2C_bus)
  writeSoundData(None, sound_data, print_data_as_columns)

  # Particle data
  # This requires the connection of a particulate sensor (zero/invalid 
  # values will be obtained if this sensor is not present).
  # Also note that, due to the low pass filtering used, the 
  # particle data become valid after an initial initialization 
  # period of approximately one minute.
  if (particle_sensor_is_on):
    particle_data = get_particle_data(I2C_bus, PARTICLE_SENSOR)
  writeParticleData(None, particle_data, print_data_as_columns)
  
  if print_data_as_columns:
    print("")
  else:
    print("-------------------------------------------")
  
  #Turn the particle sensor on/off if required 
  if (particle_sensor_is_on):
    # Stop the particle detection on the MS430
    I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [PARTICLE_SENSOR_OFF])
      
    # Turn off the hardware:
    GPIO.output(particle_sensor_control_pin, 0)
    particle_sensor_is_on = False
  else:
    particle_sensor_count += 1
    if (particle_sensor_count >= off_cycles):
      # Turn on the hardware:
      GPIO.output(particle_sensor_control_pin, 1)
      
      # Start the particle detection on the MS430
      I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [PARTICLE_SENSOR])
      
      particle_sensor_count = 0
      particle_sensor_is_on = True
  
  
