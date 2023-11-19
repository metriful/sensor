/*
  Metriful_sensor.cpp

  This file defines functions which are used in the code examples.

  Copyright 2020-2023 Metriful Ltd.
  Licensed under the MIT License - for further details see LICENSE.txt

  For code examples, datasheet and user guide, visit
  https://github.com/metriful/sensor
*/

#include "Metriful_sensor.h"
#include "host_pin_definitions.h"
#include <stdarg.h>

char strbuf[100] = {0};
bool enableSerial = true;

void SensorHardwareSetup(uint8_t i2c_7bit_address)
{
  // Turn off the LED (there may also be a power LED which you can cover or remove)
  #ifdef LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
    #ifdef ESP8266
      digitalWrite(LED_BUILTIN, HIGH);
    #else
      digitalWrite(LED_BUILTIN, LOW);
    #endif
  #endif

  #ifdef ESP8266
    // Must specify the I2C pins
    Wire.begin(SDA_PIN, SCL_PIN);
  #elif defined ARDUINO_ARCH_RP2040
    // Must specify the I2C pins
    Wire.setSDA(SDA_PIN);
    Wire.setSCL(SCL_PIN);
    Wire.begin();
  #else
    // Default I2C pins are used
    Wire.begin();
  #endif
  
  Wire.setClock(I2C_CLK_FREQ_HZ);

  // READY, light interrupt and sound interrupt lines are digital inputs.
  pinMode(READY_PIN, INPUT);
  pinMode(L_INT_PIN, INPUT);
  pinMode(S_INT_PIN, INPUT);

  // Set up interrupt monitoring of the READY signal, triggering on a
  // falling edge event (high-to-low voltage change) indicating READY
  // assertion. The function ready_ISR() will be called when this happens.
  attachInterrupt(digitalPinToInterrupt(READY_PIN), ready_ISR, FALLING);

  if (enableSerial)
  {
    // Start the serial port.
    // Full settings are: 8 data bits, no parity, one stop bit
    Serial.begin(SERIAL_BAUD_RATE);
  }
  
  // Wait for the MS430 to finish power-on initialization:
  while (digitalRead(READY_PIN) == HIGH)
  {
    yield();
  } 

  // Reset to clear any previous state:
  TransmitI2C(i2c_7bit_address, RESET_CMD, 0, 0);
  delay(5);
  
  // Wait for reset completion and entry to standby mode
  while (digitalRead(READY_PIN) == HIGH)
  {
    yield();
  } 
}

volatile bool ready_assertion_event = false;

// This function is automatically called after a falling edge
// (assertion) of READY and the flag variable is set true - it
// must be set false again in the main program.
void ISR_ATTRIBUTE ready_ISR(void)
{
  ready_assertion_event = true;
}

// Format and print a string to the serial port. Equivalent to
// printf() which is not available on Arduino. Limited to sizeof strbuf.
void s_printf(const char * format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf(strbuf, sizeof strbuf, format, args);
  va_end(args);
  Serial.print(strbuf);
}

////////////////////////////////////////////////////////////////////////

// Functions to convert data from integer representation to floating-point
// representation. Floats are easy to use for writing programs but require
// greater memory and processing power resources, so may not always be
// appropriate.

void convertAirDataF(const AirData_t * airData_in, AirData_F_t * airDataF_out)
{
  // Decode the signed value for T (in Celsius)
  airDataF_out->T_C = convertEncodedTemperatureToFloat(
                        airData_in->T_C_int_with_sign,
                        airData_in->T_C_fr_1dp);
  airDataF_out->P_Pa = airData_in->P_Pa;
  airDataF_out->H_pc = ((float) airData_in->H_pc_int)
                        + (((float) airData_in->H_pc_fr_1dp) / 10.0f);
  airDataF_out->G_Ohm = airData_in->G_ohm;
}

void convertAirQualityDataF(const AirQualityData_t * airQualityData_in, 
                                AirQualityData_F_t * airQualityDataF_out)
{
  airQualityDataF_out->AQI =  ((float) airQualityData_in->AQI_int) + 
                             (((float) airQualityData_in->AQI_fr_1dp) / 10.0f);
  airQualityDataF_out->CO2e = ((float) airQualityData_in->CO2e_int) + 
                             (((float) airQualityData_in->CO2e_fr_1dp) / 10.0f);
  airQualityDataF_out->bVOC = ((float) airQualityData_in->bVOC_int) + 
                             (((float) airQualityData_in->bVOC_fr_2dp) / 100.0f);
  airQualityDataF_out->AQI_accuracy = airQualityData_in->AQI_accuracy;
}

void convertLightDataF(const LightData_t * lightData_in,
                       LightData_F_t * lightDataF_out)
{
  lightDataF_out->illum_lux = ((float) lightData_in->illum_lux_int)
                          + (((float) lightData_in->illum_lux_fr_2dp) / 100.0f);
  lightDataF_out->white = lightData_in->white;
}

void convertSoundDataF(const SoundData_t * soundData_in,
                       SoundData_F_t * soundDataF_out)
{
  soundDataF_out->SPL_dBA = ((float) soundData_in->SPL_dBA_int)
                      + (((float) soundData_in->SPL_dBA_fr_1dp) / 10.0f);
  for (uint16_t i = 0; i < SOUND_FREQ_BANDS; i++)
  {
    soundDataF_out->SPL_bands_dB[i] = ((float) soundData_in->SPL_bands_dB_int[i])
                       + (((float) soundData_in->SPL_bands_dB_fr_1dp[i]) / 10.0f);
  }
  soundDataF_out->peakAmp_mPa = ((float) soundData_in->peak_amp_mPa_int)
                      + (((float) soundData_in->peak_amp_mPa_fr_2dp) / 100.0f);
  soundDataF_out->stable = (soundData_in->stable == 1);
}

void convertParticleDataF(const ParticleData_t * particleData_in,
                          ParticleData_F_t * particleDataF_out)
{
  particleDataF_out->duty_cycle_pc = ((float) particleData_in->duty_cycle_pc_int)
                      + (((float) particleData_in->duty_cycle_pc_fr_2dp) / 100.0f);
  particleDataF_out->concentration = ((float) particleData_in->concentration_int)
                      + (((float) particleData_in->concentration_fr_2dp) / 100.0f);
  particleDataF_out->valid = (particleData_in->valid == 1);
}

////////////////////////////////////////////////////////////////////////

// The following five functions print data (in floating-point 
// representation) over the serial port as text

void printAirDataF(const AirData_F_t * airDataF)
{
  Serial.print("Temperature = ");
  #ifdef USE_FAHRENHEIT
    float temperature_F = convertCtoF(airDataF->T_C);
    Serial.print(temperature_F, 1);
    Serial.println(" " FAHRENHEIT_SYMBOL);
  #else
    Serial.print(airDataF->T_C, 1);
    Serial.println(" " CELSIUS_SYMBOL);
  #endif
  Serial.print("Pressure = ");
  Serial.print(airDataF->P_Pa); Serial.println(" Pa");
  Serial.print("Humidity = ");
  Serial.print(airDataF->H_pc, 1); Serial.println(" %");
  Serial.print("Gas Sensor Resistance = ");
  Serial.print(airDataF->G_Ohm); Serial.println(" " OHM_SYMBOL);
}

void printAirQualityDataF(const AirQualityData_F_t * airQualityDataF)
{
  if (airQualityDataF->AQI_accuracy > 0)
  {
    Serial.print("Air Quality Index = ");
    Serial.print(airQualityDataF->AQI, 1);
    Serial.print(" (");
    Serial.print(interpret_AQI_value((uint16_t) airQualityDataF->AQI));
    Serial.println(")");
    Serial.print("Estimated CO" SUBSCRIPT_2 " = ");
    Serial.print(airQualityDataF->CO2e, 1);
    Serial.println(" ppm");
    Serial.print("Equivalent Breath VOC = ");
    Serial.print(airQualityDataF->bVOC, 2);
    Serial.println(" ppm");
  }
  Serial.print("Air Quality Accuracy: ");
  Serial.println(interpret_AQI_accuracy(airQualityDataF->AQI_accuracy));
}

void printLightDataF(const LightData_F_t * lightDataF)
{
  Serial.print("Illuminance = ");
  Serial.print(lightDataF->illum_lux, 2); Serial.println(" lux");
  Serial.print("White Light Level = ");
  Serial.println(lightDataF->white);
}

void printSoundDataF(const SoundData_F_t * soundDataF)
{
  Serial.print("A-weighted Sound Pressure Level = ");
  Serial.print(soundDataF->SPL_dBA, 1); Serial.println(" dBA");
  for (uint16_t i = 0; i < SOUND_FREQ_BANDS; i++)
  {
    s_printf("Frequency Band %u (%u Hz) SPL = ", i + 1,
             sound_band_mids_Hz[i]);
    Serial.print(soundDataF->SPL_bands_dB[i], 1); Serial.println(" dB");
  }
  Serial.print("Peak Sound Amplitude = ");
  Serial.print(soundDataF->peakAmp_mPa, 2); Serial.println(" mPa");
}

void printParticleDataF(const ParticleData_F_t * particleDataF,
                        uint8_t particleSensor)
{
  Serial.print("Particle Duty Cycle = ");
  Serial.print(particleDataF->duty_cycle_pc, 2); Serial.println(" %");
  Serial.print("Particle Concentration = ");
  Serial.print(particleDataF->concentration, 2);
  if (particleSensor == PARTICLE_SENSOR_PPD42)
  {
    Serial.println(" ppL");
  }
  else if (particleSensor == PARTICLE_SENSOR_SDS011)
  {
    Serial.println(" " SDS011_UNIT_SYMBOL);
  }
  else
  {
    Serial.println(" (?)");
  }
  Serial.print("Particle data valid: ");
  if (particleDataF->valid)
  {
    Serial.println("Yes");
  }
  else
  {
    Serial.println("No (Initializing)");
  }
}

////////////////////////////////////////////////////////////////////////

// The following five functions print data (in integer representation)
// over the serial port as text. "printColumns" determines the print format:
//  - printColumns = false gives labeled values with measurement units
//  - printColumns = true gives columns of numbers (good for spreadsheets).

void printAirData(const AirData_t * airData, bool printColumns)
{
  uint8_t T_intPart = 0;
  uint8_t T_fractionalPart = 0;
  bool isPositive = true;
  const char * T_unit = getTemperature(airData, &T_intPart,
                                       &T_fractionalPart, &isPositive);
  if (printColumns)
  {
    // temperature, pressure/Pa, humidity/%, gas sensor resistance/ohm
    s_printf("%s%u.%u %" PRIu32 " %u.%u %" PRIu32 " ",
        isPositive ? "" : "-", T_intPart, T_fractionalPart, airData->P_Pa,
        airData->H_pc_int, airData->H_pc_fr_1dp, airData->G_ohm);
  }
  else
  {
    s_printf("Temperature = %s%u.%u %s\n",
             isPositive ? "" : "-", T_intPart, T_fractionalPart, T_unit);
    s_printf("Pressure = %" PRIu32 " Pa\n", airData->P_Pa);
    s_printf("Humidity = %u.%u %%\n",
             airData->H_pc_int, airData->H_pc_fr_1dp);
    s_printf("Gas Sensor Resistance = %" PRIu32 " " OHM_SYMBOL "\n",
             airData->G_ohm);
  }
}

void printAirQualityData(const AirQualityData_t * airQualityData,
                         bool printColumns)
{
  if (printColumns)
  {
    // Print: Air Quality Index, Estimated CO2/ppm,
    //        Equivalent breath VOC/ppm, Accuracy
    s_printf("%u.%u %u.%u %u.%02u %u ", airQualityData->AQI_int,
              airQualityData->AQI_fr_1dp, airQualityData->CO2e_int,
              airQualityData->CO2e_fr_1dp, airQualityData->bVOC_int,
              airQualityData->bVOC_fr_2dp, airQualityData->AQI_accuracy);
  }
  else
  {
    if (airQualityData->AQI_accuracy > 0)
    {
      s_printf("Air Quality Index = %u.%u (%s)\n",
               airQualityData->AQI_int, airQualityData->AQI_fr_1dp,
               interpret_AQI_value(airQualityData->AQI_int));
      s_printf("Estimated CO" SUBSCRIPT_2 " = %u.%u ppm\n",
                  airQualityData->CO2e_int, airQualityData->CO2e_fr_1dp);
      s_printf("Equivalent Breath VOC = %u.%02u ppm\n",
                  airQualityData->bVOC_int, airQualityData->bVOC_fr_2dp);
    }
    s_printf("Air Quality Accuracy: %s\n",
             interpret_AQI_accuracy(airQualityData->AQI_accuracy));
  }
}

void printSoundData(const SoundData_t * soundData, bool printColumns)
{
  if (printColumns)
  {
    // Print: 
    //    Sound pressure level/dBA,
    //    Sound pressure level for frequency bands 1 to 6 (six columns),
    //    Peak sound amplitude/mPa, stability 
    s_printf("%u.%u ", soundData->SPL_dBA_int, soundData->SPL_dBA_fr_1dp);
    for (uint16_t i = 0; i < SOUND_FREQ_BANDS; i++)
    {
      s_printf("%u.%u ", soundData->SPL_bands_dB_int[i],
               soundData->SPL_bands_dB_fr_1dp[i]);
    }
    s_printf("%u.%02u %u ", soundData->peak_amp_mPa_int, 
              soundData->peak_amp_mPa_fr_2dp, soundData->stable);
  }
  else
  {
    s_printf("A-weighted Sound Pressure Level = %u.%u dBA\n", 
                soundData->SPL_dBA_int, soundData->SPL_dBA_fr_1dp);
    for (uint8_t i = 0; i < SOUND_FREQ_BANDS; i++)
    {
      s_printf("Frequency Band %u (%u Hz) SPL = %u.%u dB\n", i + 1,
               sound_band_mids_Hz[i], soundData->SPL_bands_dB_int[i],
               soundData->SPL_bands_dB_fr_1dp[i]);
    }
    s_printf("Peak Sound Amplitude = %u.%02u mPa\n", 
             soundData->peak_amp_mPa_int, soundData->peak_amp_mPa_fr_2dp);
  }
}

void printLightData(const LightData_t * lightData, bool printColumns)
{
  if (printColumns)
  {
    // Print: illuminance/lux, white level
    s_printf("%u.%02u %u ", lightData->illum_lux_int,
              lightData->illum_lux_fr_2dp, lightData->white);
  }
  else
  {
    s_printf("Illuminance = %u.%02u lux\n",
                lightData->illum_lux_int, lightData->illum_lux_fr_2dp);
    s_printf("White Light Level = %u\n", lightData->white);
  }
}

void printParticleData(const ParticleData_t * particleData,
                       bool printColumns, uint8_t particleSensor)
                       {
  if (printColumns)
  {
    // Print: duty cycle/%, concentration
    s_printf("%u.%02u %u.%02u %u ", particleData->duty_cycle_pc_int, 
              particleData->duty_cycle_pc_fr_2dp,
              particleData->concentration_int,
              particleData->concentration_fr_2dp, particleData->valid);
  }
  else
  {
    s_printf("Particle Duty Cycle = %u.%02u %%\n",
             particleData->duty_cycle_pc_int,
             particleData->duty_cycle_pc_fr_2dp);
    const char * particleUnit = "(?)";
    if (particleSensor == PARTICLE_SENSOR_PPD42)
    {
      particleUnit = "ppL";
    }
    else if (particleSensor == PARTICLE_SENSOR_SDS011)
    {
      particleUnit = SDS011_UNIT_SYMBOL;
    }
    s_printf("Particle Concentration = %u.%02u %s\n",
             particleData->concentration_int,
             particleData->concentration_fr_2dp, particleUnit);

    Serial.print("Particle data valid: ");
    if (particleData->valid == 0)
    {
      Serial.println("No (Initializing)");
    }
    else
    {
      Serial.println("Yes");
    }
  }
}

////////////////////////////////////////////////////////////////////////

// Send data to the Metriful MS430 using the I2C ("two wire") interface.
//
// Returns true on success, false on failure.
//
// dev_addr_7bit = the 7-bit I2C address of the MS430 board.
// commandRegister = the settings register/command code to be used.
// data = array containing the data to be sent.
// data_length = the number of bytes from the "data" array to be sent.
//
bool TransmitI2C(uint8_t dev_addr_7bit, uint8_t commandRegister,
                 const uint8_t * data, uint8_t data_length)
{
  if (data_length > ARDUINO_WIRE_BUFFER_LIMIT_BYTES)
  {
    // The Arduino Wire library has a limited internal buffer size
    return false;
  }

  Wire.beginTransmission(dev_addr_7bit);
  uint8_t bytesWritten = Wire.write(commandRegister);
  if (data_length > 0)
  {
    bytesWritten += Wire.write(data, data_length); 
  }
  if (bytesWritten != (data_length + 1))
  {
    return false;
  }

  return (Wire.endTransmission(true) == 0);
}

// Read data from the Metriful MS430 using the I2C ("two wire") interface.
//
// Returns true on success, false on failure.
//
// dev_addr_7bit = the 7-bit I2C address of the MS430 board.
// commandRegister = the settings register/data code to be used.
// data = array to store the received data.
// data_length = the number of bytes to read. 
//
bool ReceiveI2C(uint8_t dev_addr_7bit, uint8_t commandRegister,
                uint8_t data[], uint8_t data_length)
{
  if (data_length == 0)
  {
    // Cannot do a zero byte read
    return false;
  }

  if (data_length > ARDUINO_WIRE_BUFFER_LIMIT_BYTES)
  {
    // The Arduino Wire library has a limited internal buffer size
    return false;
  }

  Wire.beginTransmission(dev_addr_7bit);   
  Wire.write(commandRegister);  
  if (Wire.endTransmission(false) != 0)
  {
    return false;
  }

  if (Wire.requestFrom(dev_addr_7bit, data_length,
                       (uint8_t) 1) != data_length)
  {
    // Did not receive the expected number of bytes
    return false;
  }

  for (uint8_t i = 0; i < data_length; i++)
  {
    if (Wire.available() > 0)
    {
      data[i] = Wire.read();
    }
  }

  return true;
}

/////////////////////////////////////////////////////////////////////

// Provide a readable interpretation of the accuracy code for 
// the air quality measurements (applies to all air quality data) 
const char * interpret_AQI_accuracy(uint8_t AQI_accuracy_code)
{
  switch (AQI_accuracy_code)
  {
    default:
    case 0:
      return "Not yet valid, self-calibration incomplete";
    case 1:
      return "Low accuracy, self-calibration ongoing";
    case 2:
      return "Medium accuracy, self-calibration ongoing";
    case 3:
      return "High accuracy";
  }
}

const char * interpret_AQI_accuracy_brief(uint8_t AQI_accuracy_code)
{
  switch (AQI_accuracy_code)
  {
    default:
    case 0:
      return "Not yet valid";
    case 1:
      return "Low";
    case 2:
      return "Medium";
    case 3:
      return "High";
  }
}

// Provide a readable interpretation of the AQI (air quality index)  
const char * interpret_AQI_value(uint16_t AQI)
{
  if (AQI < 50) {
    return "Good";
  }
  else if (AQI < 100) {
    return "Acceptable";
  }
  else if (AQI < 150) {
    return "Substandard";
  }
  else if (AQI < 200) {
    return "Poor";
  }
  else if (AQI < 300) {
    return "Bad";
  }
  else {
    return "Very bad";
  }
}

// Set the threshold for triggering a sound interrupt.
//
// Returns true on success, false on failure.
//
// threshold_mPa = peak sound amplitude threshold in milliPascals
bool setSoundInterruptThreshold(uint8_t dev_addr_7bit,
                                uint16_t threshold_mPa)
{
  uint8_t TXdata[SOUND_INTERRUPT_THRESHOLD_BYTES] = {0};
  TXdata[0] = (uint8_t) (threshold_mPa & 0x00FF);
  TXdata[1] = (uint8_t) (threshold_mPa >> 8);
  return TransmitI2C(dev_addr_7bit, SOUND_INTERRUPT_THRESHOLD_REG,
                     TXdata, SOUND_INTERRUPT_THRESHOLD_BYTES);
}

// Set the threshold for triggering a light interrupt.
//
// Returns true on success, false on failure.
//
// The threshold value in lux units can be fractional and is formed as:
//     threshold = thres_lux_int + (thres_lux_fr_2dp/100)
//
// Threshold values are limited to MAX_LUX_VALUE.
bool setLightInterruptThreshold(uint8_t dev_addr_7bit,
                                uint16_t thres_lux_int,
                                uint8_t thres_lux_fr_2dp)
{
  uint8_t TXdata[LIGHT_INTERRUPT_THRESHOLD_BYTES] = {0};
  TXdata[0] = (uint8_t) (thres_lux_int & 0x00FF);
  TXdata[1] = (uint8_t) (thres_lux_int >> 8);
  TXdata[2] = thres_lux_fr_2dp;
  return TransmitI2C(dev_addr_7bit, LIGHT_INTERRUPT_THRESHOLD_REG,
                     TXdata, LIGHT_INTERRUPT_THRESHOLD_BYTES);
}

////////////////////////////////////////////////////////////////////////

// Convenience functions for reading data (integer representation)
//
// For each category of data (air, sound, etc.) a pointer to the data 
// struct is passed to the ReceiveI2C() function. The received byte 
// sequence fills the data struct in the correct order so that each 
// field within the struct receives the value of an environmental data
// quantity (temperature, sound level, etc.) 

SoundData_t getSoundData(uint8_t i2c_7bit_address)
{
  SoundData_t soundData = {0};
  ReceiveI2C(i2c_7bit_address, SOUND_DATA_READ,
             (uint8_t *) &soundData, SOUND_DATA_BYTES);
  return soundData;
}

AirData_t getAirData(uint8_t i2c_7bit_address)
{
  AirData_t airData = {0};
  ReceiveI2C(i2c_7bit_address, AIR_DATA_READ,
             (uint8_t *) &airData, AIR_DATA_BYTES);
  return airData;
}

LightData_t getLightData(uint8_t i2c_7bit_address)
{
  LightData_t lightData = {0};
  ReceiveI2C(i2c_7bit_address, LIGHT_DATA_READ,
             (uint8_t *) &lightData, LIGHT_DATA_BYTES);
  return lightData;
}

AirQualityData_t getAirQualityData(uint8_t i2c_7bit_address)
{
  AirQualityData_t airQualityData = {0};
  ReceiveI2C(i2c_7bit_address, AIR_QUALITY_DATA_READ,
             (uint8_t *) &airQualityData, AIR_QUALITY_DATA_BYTES);
  return airQualityData;
}

ParticleData_t getParticleData(uint8_t i2c_7bit_address)
{
  ParticleData_t particleData = {0};
  ReceiveI2C(i2c_7bit_address, PARTICLE_DATA_READ,
             (uint8_t *) &particleData, PARTICLE_DATA_BYTES);
  return particleData;
}

// Convenience functions for reading data (float representation)

SoundData_F_t getSoundDataF(uint8_t i2c_7bit_address)
{
  SoundData_F_t soundDataF = {0};
  SoundData_t soundData = getSoundData(i2c_7bit_address);
  convertSoundDataF(&soundData, &soundDataF);
  return soundDataF;
}

AirData_F_t getAirDataF(uint8_t i2c_7bit_address)
{
  AirData_F_t airDataF = {0};
  AirData_t airData = getAirData(i2c_7bit_address);
  convertAirDataF(&airData, &airDataF);
  return airDataF;
}

LightData_F_t getLightDataF(uint8_t i2c_7bit_address)
{
  LightData_F_t lightDataF = {0};
  LightData_t lightData = getLightData(i2c_7bit_address);
  convertLightDataF(&lightData, &lightDataF);
  return lightDataF;
}

AirQualityData_F_t getAirQualityDataF(uint8_t i2c_7bit_address)
{
  AirQualityData_F_t airQualityDataF = {0};
  AirQualityData_t airQualityData = getAirQualityData(i2c_7bit_address);
  convertAirQualityDataF(&airQualityData, &airQualityDataF);
  return airQualityDataF;
}

ParticleData_F_t getParticleDataF(uint8_t i2c_7bit_address)
{
  ParticleData_F_t particleDataF = {0};
  ParticleData_t particleData = getParticleData(i2c_7bit_address);
  convertParticleDataF(&particleData, &particleDataF);
  return particleDataF;
}

////////////////////////////////////////////////////////////////////////

// Functions to convert Celsius temperature to Fahrenheit, in float 
// and integer formats

float convertCtoF(float C)
{
  return ((C * 1.8f) + 32.0f);
}

// Convert Celsius to Fahrenheit in sign, integer and fractional parts
void convertCtoF_int(float C, uint8_t * F_int, uint8_t * F_fr_1dp,
                     bool * isPositive)
{
  float F = convertCtoF(C);
  bool isNegative = (F < 0.0f);
  if (isNegative)
  {
    F = -F;
  }
  F += 0.05f;
  F_int[0] = (uint8_t) F;
  F -= (float) F_int[0];
  F_fr_1dp[0] = (uint8_t) (F * 10.0f);
  isPositive[0] = (!isNegative);
}

// Decode and convert the temperature as read from the MS430 (integer
// representation) into a float value 
float convertEncodedTemperatureToFloat(uint8_t T_C_int_with_sign,
                                       uint8_t T_C_fr_1dp)
{
  float temperature_C = ((float) (T_C_int_with_sign & TEMPERATURE_VALUE_MASK))
                          + (((float) T_C_fr_1dp) / 10.0f);
  if ((T_C_int_with_sign & TEMPERATURE_SIGN_MASK) != 0)
  {
    // The most-significant bit is set, which indicates that
    // the temperature is negative
    temperature_C = -temperature_C;
  }
  return temperature_C;
}

// Obtain temperature, in chosen units (C or F), as sign, integer
// and fractional parts
const char * getTemperature(const AirData_t * pAirData, uint8_t * T_intPart, 
                            uint8_t * T_fractionalPart, bool * isPositive)
{
  #ifdef USE_FAHRENHEIT
    float temperature_C = convertEncodedTemperatureToFloat(
                              pAirData->T_C_int_with_sign, 
                              pAirData->T_C_fr_1dp);
    convertCtoF_int(temperature_C, T_intPart, T_fractionalPart, isPositive);
    return FAHRENHEIT_SYMBOL;
  #else
    isPositive[0] = ((pAirData->T_C_int_with_sign
                      & TEMPERATURE_SIGN_MASK) == 0);
    T_intPart[0] = pAirData->T_C_int_with_sign & TEMPERATURE_VALUE_MASK;
    T_fractionalPart[0] = pAirData->T_C_fr_1dp;
    return CELSIUS_SYMBOL;
  #endif
}
