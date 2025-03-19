"""
  sensor.py

  This file creates an interface so that the MS430 Arduino code can
  be used as an external component in ESPHome.

  Suitable for ESP8266, ESP32 and Raspberry Pi Pico W.

  Copyright 2025 Metriful Ltd.
  Licensed under the MIT License - for further details see LICENSE.txt

  For code examples, datasheet and user guide, visit
  https://github.com/metriful/sensor
"""

from esphome import codegen
import esphome.config_validation as cv
from esphome.components import sensor
from esphome import const as c

CONF_WHITE_LIGHT = 'white_light'
CONF_AQI = 'aqi'
CONF_E_CO2 = 'e_co2'
CONF_B_VOC = 'b_voc'
CONF_PARTICLE_DUTY = 'particle_duty'
CONF_PARTICLE_CONC = 'particle_concentration'
CONF_SOUND_SPL = 'sound_spl'
CONF_SOUND_PEAK = 'sound_peak'
UNIT_DECIBEL_A = "dBA"
UNIT_MILLIPASCAL = "mPa"
CONF_SOUND_BAND_0 = 'sound_band_0'
CONF_SOUND_BAND_1 = 'sound_band_1'
CONF_SOUND_BAND_2 = 'sound_band_2'
CONF_SOUND_BAND_3 = 'sound_band_3'
CONF_SOUND_BAND_4 = 'sound_band_4'
CONF_SOUND_BAND_5 = 'sound_band_5'

ns = codegen.esphome_ns.namespace("ms430")
MS430 = ns.class_("MS430", codegen.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MS430),
        cv.Optional(c.CONF_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_TEMPERATURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(c.CONF_HUMIDITY): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_PERCENT,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_HUMIDITY,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(c.CONF_PRESSURE): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_PASCAL,
            accuracy_decimals=0,
            device_class=c.DEVICE_CLASS_ATMOSPHERIC_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(c.CONF_GAS_RESISTANCE): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_OHM,
            accuracy_decimals=0,
            device_class=c.DEVICE_CLASS_AQI,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_WHITE_LIGHT): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=c.DEVICE_CLASS_ILLUMINANCE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(c.CONF_ILLUMINANCE): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_LUX,
            accuracy_decimals=2,
            device_class=c.DEVICE_CLASS_ILLUMINANCE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(c.CONF_IAQ_ACCURACY): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_EMPTY,
            accuracy_decimals=0,
            device_class=c.DEVICE_CLASS_EMPTY,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_AQI): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_EMPTY,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_AQI,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_E_CO2): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_PARTS_PER_MILLION,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_GAS,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_B_VOC): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_PARTS_PER_MILLION,
            accuracy_decimals=2,
            device_class=c.DEVICE_CLASS_VOLATILE_ORGANIC_COMPOUNDS,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PARTICLE_DUTY): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_PERCENT,
            accuracy_decimals=2,
            device_class=c.DEVICE_CLASS_EMPTY,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_PARTICLE_CONC): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_MICROGRAMS_PER_CUBIC_METER,
            accuracy_decimals=2,
            device_class=c.DEVICE_CLASS_EMPTY,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOUND_SPL): sensor.sensor_schema(
            unit_of_measurement=UNIT_DECIBEL_A,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_SOUND_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOUND_PEAK): sensor.sensor_schema(
            unit_of_measurement=UNIT_MILLIPASCAL,
            accuracy_decimals=2,
            device_class=c.DEVICE_CLASS_SOUND_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOUND_BAND_0): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_DECIBEL,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_SOUND_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOUND_BAND_1): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_DECIBEL,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_SOUND_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOUND_BAND_2): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_DECIBEL,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_SOUND_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOUND_BAND_3): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_DECIBEL,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_SOUND_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOUND_BAND_4): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_DECIBEL,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_SOUND_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOUND_BAND_5): sensor.sensor_schema(
            unit_of_measurement=c.UNIT_DECIBEL,
            accuracy_decimals=1,
            device_class=c.DEVICE_CLASS_SOUND_PRESSURE,
            state_class=c.STATE_CLASS_MEASUREMENT,
        )
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = codegen.new_Pvariable(config[c.CONF_ID])
    await codegen.register_component(var, config)

    if c.CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[c.CONF_TEMPERATURE])
        codegen.add(var.set_temperature_s(sens))
    if c.CONF_HUMIDITY in config:
        sens = await sensor.new_sensor(config[c.CONF_HUMIDITY])
        codegen.add(var.set_humidity_s(sens))
    if c.CONF_PRESSURE in config:
        sens = await sensor.new_sensor(config[c.CONF_PRESSURE])
        codegen.add(var.set_pressure_s(sens))
    if c.CONF_GAS_RESISTANCE in config:
        sens = await sensor.new_sensor(config[c.CONF_GAS_RESISTANCE])
        codegen.add(var.set_gas_s(sens))
    if CONF_WHITE_LIGHT in config:
        sens = await sensor.new_sensor(config[CONF_WHITE_LIGHT])
        codegen.add(var.set_w_light_s(sens))
    if c.CONF_ILLUMINANCE in config:
        sens = await sensor.new_sensor(config[c.CONF_ILLUMINANCE])
        codegen.add(var.set_illum_s(sens))
    if c.CONF_IAQ_ACCURACY in config:
        sens = await sensor.new_sensor(config[c.CONF_IAQ_ACCURACY])
        codegen.add(var.set_aqi_acc_s(sens))
    if CONF_AQI in config:
        sens = await sensor.new_sensor(config[CONF_AQI])
        codegen.add(var.set_aqi_s(sens))
    if CONF_E_CO2 in config:
        sens = await sensor.new_sensor(config[CONF_E_CO2])
        codegen.add(var.set_CO2e_s(sens))
    if CONF_B_VOC in config:
        sens = await sensor.new_sensor(config[CONF_B_VOC])
        codegen.add(var.set_bVOC_s(sens))
    if CONF_PARTICLE_DUTY in config:
        sens = await sensor.new_sensor(config[CONF_PARTICLE_DUTY])
        codegen.add(var.set_particle_duty_s(sens))
    if CONF_PARTICLE_CONC in config:
        sens = await sensor.new_sensor(config[CONF_PARTICLE_CONC])
        codegen.add(var.set_particle_conc_s(sens))
    if CONF_SOUND_SPL in config:
        sens = await sensor.new_sensor(config[CONF_SOUND_SPL])
        codegen.add(var.set_sound_spl_s(sens))
    if CONF_SOUND_PEAK in config:
        sens = await sensor.new_sensor(config[CONF_SOUND_PEAK])
        codegen.add(var.set_sound_peak_s(sens))
    if CONF_SOUND_BAND_0 in config:
        sens = await sensor.new_sensor(config[CONF_SOUND_BAND_0])
        codegen.add(var.set_sound_band0_s(sens))
    if CONF_SOUND_BAND_1 in config:
        sens = await sensor.new_sensor(config[CONF_SOUND_BAND_1])
        codegen.add(var.set_sound_band1_s(sens))
    if CONF_SOUND_BAND_2 in config:
        sens = await sensor.new_sensor(config[CONF_SOUND_BAND_2])
        codegen.add(var.set_sound_band2_s(sens))
    if CONF_SOUND_BAND_3 in config:
        sens = await sensor.new_sensor(config[CONF_SOUND_BAND_3])
        codegen.add(var.set_sound_band3_s(sens))
    if CONF_SOUND_BAND_4 in config:
        sens = await sensor.new_sensor(config[CONF_SOUND_BAND_4])
        codegen.add(var.set_sound_band4_s(sens))
    if CONF_SOUND_BAND_5 in config:
        sens = await sensor.new_sensor(config[CONF_SOUND_BAND_5])
        codegen.add(var.set_sound_band5_s(sens))
