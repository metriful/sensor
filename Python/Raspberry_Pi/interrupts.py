#  interrupts.py

#  Example code for using the Metriful MS430 interrupt outputs. 
#  This example is designed to run with Python 3 on a Raspberry Pi.
   
#  Light and sound interrupts are configured and the program then 
#  waits indefinitely. When an interrupt occurs, a message is 
#  displayed, the interrupt is cleared (if set to latch type), 
#  and the program returns to waiting.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

from sensor_package.sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# Light level interrupt settings.

enable_light_interrupts = True
light_int_type = LIGHT_INT_TYPE_LATCH
# Choose the interrupt polarity: trigger when level rises above 
# threshold (positive), or when level falls below threshold (negative).
light_int_polarity = LIGHT_INT_POL_POSITIVE
light_thres_lux_i = 100
light_thres_lux_f2dp = 50
# The interrupt threshold value in lux units can be fractional and is formed as:
#     threshold = light_thres_lux_i + (light_thres_lux_f2dp/100)
# E.g. for a light threshold of 56.12 lux, set:
#     light_thres_lux_i = 56
#     light_thres_lux_f2dp = 12 
  
# Sound level interrupt settings.

enable_sound_interrupts = True
sound_int_type = SOUND_INT_TYPE_LATCH 
sound_thres_mPa = 100

# END OF USER-EDITABLE SETTINGS
#########################################################

if ((light_thres_lux_i + (float(light_thres_lux_f2dp)/100.0)) > MAX_LUX_VALUE):
  raise Exception("The chosen light interrupt threshold exceeds the "
                  "maximum allowed value of " + str(MAX_LUX_VALUE) + " lux")

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = SensorHardwareSetup()

#########################################################

if (enable_sound_interrupts):
  # Set the interrupt type (latch or comparator)
  I2C_bus.write_i2c_block_data(i2c_7bit_address, SOUND_INTERRUPT_TYPE_REG, [sound_int_type])
  
  # Set the threshold
  setSoundInterruptThreshold(I2C_bus, sound_thres_mPa)
  
  # Tell the Pi to monitor the interrupt line for a falling edge event (high-to-low voltage change)
  GPIO.add_event_detect(sound_int_pin, GPIO.FALLING) 
  
  # Enable the interrupt on the MS430
  I2C_bus.write_i2c_block_data(i2c_7bit_address, SOUND_INTERRUPT_ENABLE_REG, [ENABLED])


if (enable_light_interrupts):
  # Set the interrupt type (latch or comparator)
  I2C_bus.write_i2c_block_data(i2c_7bit_address, LIGHT_INTERRUPT_TYPE_REG, [light_int_type])
  
  # Set the threshold
  setLightInterruptThreshold(I2C_bus, light_thres_lux_i, light_thres_lux_f2dp)
  
  # Set the interrupt polarity
  I2C_bus.write_i2c_block_data(i2c_7bit_address, LIGHT_INTERRUPT_POLARITY_REG, [light_int_polarity])
  
  # Tell the Pi to monitor the interrupt line for a falling edge event (high-to-low voltage change)
  GPIO.add_event_detect(light_int_pin, GPIO.FALLING) 
  
  # Enable the interrupt on the MS430
  I2C_bus.write_i2c_block_data(i2c_7bit_address, LIGHT_INTERRUPT_ENABLE_REG, [ENABLED])


if (not enable_light_interrupts) and (not enable_sound_interrupts):
  print("No interrupts have been enabled. Press ctrl-c to exit.")
else:  
  print("Waiting for interrupts. Press ctrl-c to exit.")
  print("")


while (True):

  # Check whether a light interrupt has occurred
  if (GPIO.event_detected(light_int_pin) and enable_light_interrupts):
    print("LIGHT INTERRUPT.")
    if (light_int_type == LIGHT_INT_TYPE_LATCH):
      # Latch type interrupts remain set until cleared by command
      I2C_bus.write_byte(i2c_7bit_address, LIGHT_INTERRUPT_CLR_CMD)

  # Check whether a sound interrupt has occurred   
  if (GPIO.event_detected(sound_int_pin) and enable_sound_interrupts):
    print("SOUND INTERRUPT.")
    if (sound_int_type == SOUND_INT_TYPE_LATCH):
      # Latch type interrupts remain set until cleared by command
      I2C_bus.write_byte(i2c_7bit_address, SOUND_INTERRUPT_CLR_CMD)
      
  sleep(0.5)


