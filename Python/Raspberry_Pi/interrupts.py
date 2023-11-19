"""Example of using the Metriful MS430 interrupt outputs, from a Raspberry Pi.

Light and sound interrupts are configured and the program then
waits indefinitely. When an interrupt occurs, a message is
displayed, the interrupt is cleared (if set to latch type),
and the program returns to waiting.
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

import time
import sensor_package.sensor_functions as sensor
import sensor_package.sensor_constants as const

#########################################################
# USER-EDITABLE SETTINGS

enable_light_interrupts = True
light_int_type = const.LIGHT_INT_TYPE_LATCH

# Choose the light interrupt polarity. The interrupt triggers when
# the light level rises above the threshold (positive), or when
# the level falls below the threshold (negative).
light_int_polarity = const.LIGHT_INT_POL_POSITIVE
light_thres_lux_i = 100
light_thres_lux_f2dp = 50
# The interrupt threshold in lux units can be fractional and is formed as:
#     threshold = light_thres_lux_i + (light_thres_lux_f2dp/100)
# E.g. for a light threshold of 56.12 lux, set:
#     light_thres_lux_i = 56
#     light_thres_lux_f2dp = 12

enable_sound_interrupts = True
sound_int_type = const.SOUND_INT_TYPE_LATCH
sound_thres_mPa = 100

# END OF USER-EDITABLE SETTINGS
#########################################################

if ((light_thres_lux_i
     + (float(light_thres_lux_f2dp)/100.0)) > const.MAX_LUX_VALUE):
    raise ValueError("The chosen light interrupt threshold exceeds the "
                     f"maximum allowed value of {const.MAX_LUX_VALUE} lux")

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = sensor.SensorHardwareSetup()

#########################################################

if enable_sound_interrupts:
    # Set the interrupt type (latch or comparator)
    I2C_bus.write_i2c_block_data(
        sensor.i2c_7bit_address,
        const.SOUND_INTERRUPT_TYPE_REG, [sound_int_type])

    # Set the threshold
    sensor.setSoundInterruptThreshold(I2C_bus, sound_thres_mPa)

    # Tell the Pi to monitor the interrupt line for a falling
    # edge event (high-to-low voltage change)
    GPIO.add_event_detect(sensor.sound_int_pin, GPIO.FALLING)

    # Enable the interrupt on the MS430
    I2C_bus.write_i2c_block_data(
        sensor.i2c_7bit_address,
        const.SOUND_INTERRUPT_ENABLE_REG, [const.ENABLED])


if enable_light_interrupts:
    # Set the interrupt type (latch or comparator)
    I2C_bus.write_i2c_block_data(
        sensor.i2c_7bit_address,
        const.LIGHT_INTERRUPT_TYPE_REG, [light_int_type])

    # Set the threshold
    sensor.setLightInterruptThreshold(
        I2C_bus, light_thres_lux_i, light_thres_lux_f2dp)

    # Set the interrupt polarity
    I2C_bus.write_i2c_block_data(
        sensor.i2c_7bit_address,
        const.LIGHT_INTERRUPT_POLARITY_REG, [light_int_polarity])

    # Tell the Pi to monitor the interrupt line for a falling
    # edge event (high-to-low voltage change)
    GPIO.add_event_detect(sensor.light_int_pin, GPIO.FALLING)

    # Enable the interrupt on the MS430
    I2C_bus.write_i2c_block_data(
        sensor.i2c_7bit_address,
        const.LIGHT_INTERRUPT_ENABLE_REG, [const.ENABLED])


if (not enable_light_interrupts) and (not enable_sound_interrupts):
    print("No interrupts have been enabled. Press ctrl-c to exit.")
else:
    print("Waiting for interrupts. Press ctrl-c to exit.")
    print("")


while True:

    # Check whether a light interrupt has occurred
    if GPIO.event_detected(sensor.light_int_pin) and enable_light_interrupts:
        print("LIGHT INTERRUPT.")
        if (light_int_type == const.LIGHT_INT_TYPE_LATCH):
            # Latch type interrupts remain set until cleared by command
            I2C_bus.write_byte(sensor.i2c_7bit_address,
                               const.LIGHT_INTERRUPT_CLR_CMD)

    # Check whether a sound interrupt has occurred
    if GPIO.event_detected(sensor.sound_int_pin) and enable_sound_interrupts:
        print("SOUND INTERRUPT.")
        if (sound_int_type == const.SOUND_INT_TYPE_LATCH):
            # Latch type interrupts remain set until cleared by command
            I2C_bus.write_byte(sensor.i2c_7bit_address,
                               const.SOUND_INTERRUPT_CLR_CMD)

    time.sleep(0.5)
