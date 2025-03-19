/*
  ms430_esphome.cpp

  This file creates an interface so that the MS430 Arduino code can
  be used as an external component in ESPHome.

  Suitable for ESP8266, ESP32 and Raspberry Pi Pico W.

  Copyright 2025 Metriful Ltd.
  Licensed under the MIT License - for further details see LICENSE.txt

  For code examples, datasheet and user guide, visit
  https://github.com/metriful/sensor
*/

#include "ms430_esphome.h"
#include "esphome/core/helpers.h"
#include "Metriful_sensor.h"
#include "host_pin_definitions.h"
#include "sensor_constants.h"

namespace esphome {
namespace ms430 {

void MS430::setup()
{
    enableSerial = false;
    SensorHardwareSetup(I2C_ADDRESS);
    uint8_t particleSensor = PARTICLE_SENSOR;
    TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, &particleSensor, 1);
    uint8_t cyclePeriod = CYCLE_PERIOD;
    TransmitI2C(I2C_ADDRESS, CYCLE_TIME_PERIOD_REG, &cyclePeriod, 1);
    ready_assertion_event = false;
    TransmitI2C(I2C_ADDRESS, CYCLE_MODE_CMD, 0, 0);
}

void MS430::loop()
{
    static uint8_t stage = 0;
    if (ready_assertion_event)
    {
        ready_assertion_event = false;
        if (stage == 0)
        {
            stage = 1;
        }
    }
    stage = this->output(stage);
}

float MS430::get_setup_priority() const
{
    return setup_priority::BUS;
}

uint8_t MS430::output(uint8_t stage)
{
    static AirData_F_t airDataF;
    static AirQualityData_F_t airQualityDataF;
    static ParticleData_F_t particleDataF;
    static LightData_F_t lightDataF;
    static SoundData_F_t soundDataF;

    if (stage == 0)
    {
        return 0;
    }
    if (stage == 1)
    {
        airDataF = getAirDataF(I2C_ADDRESS);
    }
    if (stage == 2)
    {
        airQualityDataF = getAirQualityDataF(I2C_ADDRESS);
    }
    if ((stage == 3) && (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF))
    {
        particleDataF = getParticleDataF(I2C_ADDRESS);
    }
    if (stage == 4)
    {
        lightDataF = getLightDataF(I2C_ADDRESS);
    }
    if (stage == 5)
    {
        soundDataF = getSoundDataF(I2C_ADDRESS);
    }
    if (stage == 6)
    {
        if (this->temperature_sensor != nullptr) {
            this->temperature_sensor->publish_state(airDataF.T_C);
        }
        if (this->pressure_sensor != nullptr) {
            this->pressure_sensor->publish_state(airDataF.P_Pa);
        }
        if (this->humidity_sensor != nullptr) {
            this->humidity_sensor->publish_state(airDataF.H_pc);
        }
        if (this->gas_sensor != nullptr) {
            this->gas_sensor->publish_state(airDataF.G_Ohm);
        }
    }
    if (stage == 7)
    {
        // Only publish air quality values when the algorithm has
        // initialized.
        if (this->aqi_acc_sensor != nullptr) {
            this->aqi_acc_sensor->publish_state(airQualityDataF.AQI_accuracy);
        }
        if (airQualityDataF.AQI_accuracy > 0)
        {
            AQIinitialized = true;
        }

        if (AQIinitialized)
        {
            if (this->aqi_sensor != nullptr) {
                this->aqi_sensor->publish_state(airQualityDataF.AQI);
            }
            if (this->CO2e_sensor != nullptr) {
                this->CO2e_sensor->publish_state(airQualityDataF.CO2e);
            }
            if (this->bVOC_sensor != nullptr) {
                this->bVOC_sensor->publish_state(airQualityDataF.bVOC);
            }
        }
    }
    if ((stage == 8) && (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF))
    {
        if (this->particle_duty_sensor != nullptr) {
            this->particle_duty_sensor->publish_state(particleDataF.duty_cycle_pc);
        }
        if (this->particle_conc_sensor != nullptr) {
            this->particle_conc_sensor->publish_state(particleDataF.concentration);
        }
    }
    if (stage == 9)
    {
        if (this->w_light_sensor != nullptr) {
            this->w_light_sensor->publish_state(lightDataF.white);
        }
        if (this->illum_sensor != nullptr) {
            this->illum_sensor->publish_state(lightDataF.illum_lux);
        }
    }
    if (stage == 10)
    {
        if (this->sound_spl_sensor != nullptr) {
            this->sound_spl_sensor->publish_state(soundDataF.SPL_dBA);
        }
        if (this->sound_peak_sensor != nullptr) {
            this->sound_peak_sensor->publish_state(soundDataF.peakAmp_mPa);
        }
        if (this->sound_band0_sensor != nullptr) {
            this->sound_band0_sensor->publish_state(soundDataF.SPL_bands_dB[0]);
        }
        if (this->sound_band1_sensor != nullptr) {
            this->sound_band1_sensor->publish_state(soundDataF.SPL_bands_dB[1]);
        }
    }
    if (stage == 11)
    {
        if (this->sound_band2_sensor != nullptr) {
            this->sound_band2_sensor->publish_state(soundDataF.SPL_bands_dB[2]);
        }
        if (this->sound_band3_sensor != nullptr) {
            this->sound_band3_sensor->publish_state(soundDataF.SPL_bands_dB[3]);
        }
        if (this->sound_band4_sensor != nullptr) {
            this->sound_band4_sensor->publish_state(soundDataF.SPL_bands_dB[4]);
        }
        if (this->sound_band5_sensor != nullptr) {
            this->sound_band5_sensor->publish_state(soundDataF.SPL_bands_dB[5]);
        }
    }
    stage++;
    if (stage > 11) {
        stage = 0;
    }
    this->status_clear_warning();
    return stage;
}

}
}
