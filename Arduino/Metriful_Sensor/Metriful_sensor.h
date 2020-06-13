/* 
   Metriful_sensor.h

   This file declares functions which are used in the Metriful code examples.
   The function definitions are in file Metriful_sensor.cpp

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit https://github.com/metriful/sensor
*/

#ifndef METRIFUL_SENSOR_H
#define METRIFUL_SENSOR_H

#include "Arduino.h"
#include <stdint.h>
#include "sensor_constants.h"
#include "Arduino_pin_definitions.h"

#define I2C_CLK_FREQ_HZ 100000
#define SERIAL_BAUD_RATE 9600

extern volatile bool ready_assertion_event; 

////////////////////////////////////////////////////////////////////////

// Data category structs containing floats. If floats are not wanted, 
// use the integer-only struct versions in sensor_constants.h 

typedef struct {
  float SPL_dBA;
  float SPL_bands_dB[SOUND_FREQ_BANDS];
  float peakAmp_mPa;
  bool  stable;
} SoundData_F_t;

typedef struct {
  float    T_C;
  uint32_t P_Pa;
  float    H_pc;
  uint32_t G_Ohm;
} AirData_F_t;

typedef struct {
  float   AQI;
  float   CO2e;
  float   bVOC;
  uint8_t AQI_accuracy;
} AirQualityData_F_t;

typedef struct {
  float    illum_lux;
  uint16_t white;
} LightData_F_t;

typedef struct {
  float    occupancy_pc;
  uint16_t concentration_ppL;
} ParticleData_F_t;

////////////////////////////////////////////////////////////////////////

void SensorHardwareSetup(uint8_t i2c_7bit_address);
void ready_ISR(void);

bool TransmitI2C(uint8_t dev_addr_7bit, uint8_t commandRegister, uint8_t data[], uint8_t data_length);
bool ReceiveI2C(uint8_t dev_addr_7bit, uint8_t commandRegister, uint8_t data[], uint8_t data_length);

const char * interpret_AQI_accuracy(uint8_t AQI_accuracy_code);
const char * interpret_AQI_value(uint16_t AQI);

void convertAirDataF(const AirData_t * airData_in, AirData_F_t * airDataF_out);
void convertAirQualityDataF(const AirQualityData_t * airQualityData_in, 
                            AirQualityData_F_t * airQualityDataF_out);
void convertLightDataF(const LightData_t * lightData_in, LightData_F_t * lightDataF_out);
void convertSoundDataF(const SoundData_t * soundData_in, SoundData_F_t * soundDataF_out);
void convertParticleDataF(const ParticleData_t * particleData_in, ParticleData_F_t * particleDataF_out);

void printAirDataF(const AirData_F_t * airDataF);
void printAirQualityDataF(const AirQualityData_F_t * airQualityDataF);
void printLightDataF(const LightData_F_t * lightDataF);
void printSoundDataF(const SoundData_F_t * soundDataF);
void printParticleDataF(const ParticleData_F_t * particleDataF);

void printAirData(const AirData_t * airData, bool printColumns);
void printAirQualityData(const AirQualityData_t * airQualityData, bool printColumns);
void printLightData(const LightData_t * lightData, bool printColumns);
void printSoundData(const SoundData_t * soundData, bool printColumns);
void printParticleData(const ParticleData_t * particleData, bool printColumns);

#endif
