/*
  ms430_esphome.h

  This file creates an interface so that the MS430 Arduino code can
  be used as an external component in ESPHome.

  Suitable for ESP8266, ESP32 and Raspberry Pi Pico W.

  Copyright 2025 Metriful Ltd.
  Licensed under the MIT License - for further details see LICENSE.txt

  For code examples, datasheet and user guide, visit
  https://github.com/metriful/sensor
*/

#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"

// Choose time interval for reading data (every 3, 100, or 300 seconds)
// 100 or 300 seconds are recommended to avoid self-heating.
#define CYCLE_PERIOD CYCLE_PERIOD_100_S

//////////////////////////////////////////////////////////////

extern bool enableSerial;

namespace esphome {
namespace ms430 {

class MS430 : public Component {
    public:
        void set_temperature_s(sensor::Sensor *s) { temperature_sensor = s; }
        void set_pressure_s(sensor::Sensor *s) { pressure_sensor = s; }
        void set_humidity_s(sensor::Sensor *s) { humidity_sensor = s; }
        void set_gas_s(sensor::Sensor *s) { gas_sensor = s; }
        void set_w_light_s(sensor::Sensor *s) { w_light_sensor = s; }
        void set_illum_s(sensor::Sensor *s) { illum_sensor = s; }
        void set_aqi_acc_s(sensor::Sensor *s) { aqi_acc_sensor = s; }
        void set_aqi_s(sensor::Sensor *s) { aqi_sensor = s; }
        void set_CO2e_s(sensor::Sensor *s) { CO2e_sensor = s; }
        void set_bVOC_s(sensor::Sensor *s) { bVOC_sensor = s; }
        void set_particle_duty_s(sensor::Sensor *s) { particle_duty_sensor = s; }
        void set_particle_conc_s(sensor::Sensor *s) { particle_conc_sensor = s; }
        void set_sound_spl_s(sensor::Sensor *s) { sound_spl_sensor = s; }
        void set_sound_peak_s(sensor::Sensor *s) { sound_peak_sensor = s; }
        void set_sound_band0_s(sensor::Sensor *s) { sound_band0_sensor = s; }
        void set_sound_band1_s(sensor::Sensor *s) { sound_band1_sensor = s; }
        void set_sound_band2_s(sensor::Sensor *s) { sound_band2_sensor = s; }
        void set_sound_band3_s(sensor::Sensor *s) { sound_band3_sensor = s; }
        void set_sound_band4_s(sensor::Sensor *s) { sound_band4_sensor = s; }
        void set_sound_band5_s(sensor::Sensor *s) { sound_band5_sensor = s; }

        void setup() override;
        void loop() override;
        float get_setup_priority() const override;

    protected:
        uint8_t output(uint8_t stage);
        sensor::Sensor *temperature_sensor{nullptr};
        sensor::Sensor *pressure_sensor{nullptr};
        sensor::Sensor *humidity_sensor{nullptr};
        sensor::Sensor *gas_sensor{nullptr};
        sensor::Sensor *w_light_sensor{nullptr};
        sensor::Sensor *illum_sensor{nullptr};
        sensor::Sensor *aqi_acc_sensor{nullptr};
        sensor::Sensor *aqi_sensor{nullptr};
        sensor::Sensor *CO2e_sensor{nullptr};
        sensor::Sensor *bVOC_sensor{nullptr};
        sensor::Sensor *particle_duty_sensor{nullptr};
        sensor::Sensor *particle_conc_sensor{nullptr};
        sensor::Sensor *sound_spl_sensor{nullptr};
        sensor::Sensor *sound_peak_sensor{nullptr};
        sensor::Sensor *sound_band0_sensor{nullptr};
        sensor::Sensor *sound_band1_sensor{nullptr};
        sensor::Sensor *sound_band2_sensor{nullptr};
        sensor::Sensor *sound_band3_sensor{nullptr};
        sensor::Sensor *sound_band4_sensor{nullptr};
        sensor::Sensor *sound_band5_sensor{nullptr};
        bool AQIinitialized = false;
};

}
}
