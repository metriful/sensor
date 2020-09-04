/* 
   Metriful_sensor.cpp

   This file defines functions which are used in the code examples.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include "Metriful_sensor.h"
#include "host_pin_definitions.h"

#ifdef ARDUINO_SAMD_NANO_33_IOT
  // The Arduino Nano 33 IoT wifi module prevents the use of the 
  // built-in hardware-supported I2C module, so a software I2C library
  // must be used instead. SlowSoftWire is one of several libraries available.
  SlowSoftWire TheWire(SOFT_SDA, SOFT_SCL, false);
#endif

// The Arduino Wire library has a limited internal buffer size:
#define ARDUINO_WIRE_BUFFER_LIMIT_BYTES 32

void SensorHardwareSetup(uint8_t i2c_7bit_address) {

  pinMode(LED_BUILTIN, OUTPUT);

  #ifdef ESP8266
    // Must specify the I2C pins
    TheWire.begin(SDA_PIN, SCL_PIN); 
    digitalWrite(LED_BUILTIN, HIGH);
  #else
    TheWire.begin(); 
    digitalWrite(LED_BUILTIN, LOW);
  #endif
  
  TheWire.setClock(I2C_CLK_FREQ_HZ);

  // READY, light interrupt and sound interrupt lines are digital inputs.
  pinMode(READY_PIN, INPUT);
  pinMode(L_INT_PIN, INPUT);
  pinMode(S_INT_PIN, INPUT);

  // Set up interrupt monitoring of the READY signal, triggering on a falling edge 
  // event (high-to-low voltage change) indicating READY assertion. The
  // function ready_ISR() will be called when this happens.
  attachInterrupt(digitalPinToInterrupt(READY_PIN), ready_ISR, FALLING);

  // Start the serial port.
  // Full settings are: 8 data bits, no parity, one stop bit
  Serial.begin(SERIAL_BAUD_RATE);
  
  // Wait for the MS430 to finish power-on initialization:
  while (digitalRead(READY_PIN) == HIGH) {
    yield();
  } 
  
  // Reset to clear any previous state:
  TransmitI2C(i2c_7bit_address, RESET_CMD, 0, 0);
  delay(5);
  
  // Wait for reset completion and entry to standby mode
  while (digitalRead(READY_PIN) == HIGH) {
    yield();
  } 
}

volatile bool ready_assertion_event = false;

// This function is automatically called after a falling edge (assertion) of READY.
// The flag variable is set true - it must be set false again in the main program.
#ifdef ESP8266
void ICACHE_RAM_ATTR ready_ISR(void) {
  ready_assertion_event = true;
}
#else
void ready_ISR(void) {
  ready_assertion_event = true;
}
#endif

////////////////////////////////////////////////////////////////////////

// Functions to convert data from integer representation to floating-point representation.
// Floats are easy to use for writing programs but require greater memory and processing
// power resources, so may not always be appropriate.

void convertAirDataF(const AirData_t * airData_in, AirData_F_t * airDataF_out) {
  // Decode the signed value for T
  float absoluteValue = ((float) (airData_in->T_C_int_with_sign & TEMPERATURE_VALUE_MASK)) + 
                       (((float) airData_in->T_C_fr_1dp)/10.0);
  if ((airData_in->T_C_int_with_sign & TEMPERATURE_SIGN_MASK) != 0) {
    // the most-significant bit is set, indicating that the temperature is negative
    airDataF_out->T_C = -absoluteValue;
  }
  else {
    // temperature is positive
    airDataF_out->T_C = absoluteValue;
  }
  airDataF_out->P_Pa = airData_in->P_Pa;
  airDataF_out->H_pc = ((float) airData_in->H_pc_int) + (((float) airData_in->H_pc_fr_1dp)/10.0);
  airDataF_out->G_Ohm = airData_in->G_ohm;
}

void convertAirQualityDataF(const AirQualityData_t * airQualityData_in, 
                                AirQualityData_F_t * airQualityDataF_out) {
  airQualityDataF_out->AQI =  ((float) airQualityData_in->AQI_int) + 
                             (((float) airQualityData_in->AQI_fr_1dp)/10.0);
  airQualityDataF_out->CO2e = ((float) airQualityData_in->CO2e_int) + 
                             (((float) airQualityData_in->CO2e_fr_1dp)/10.0);
  airQualityDataF_out->bVOC = ((float) airQualityData_in->bVOC_int) + 
                             (((float) airQualityData_in->bVOC_fr_2dp)/100.0);
  airQualityDataF_out->AQI_accuracy = airQualityData_in->AQI_accuracy;
}

void convertLightDataF(const LightData_t * lightData_in, LightData_F_t * lightDataF_out) {
  lightDataF_out->illum_lux = ((float) lightData_in->illum_lux_int) + 
                             (((float) lightData_in->illum_lux_fr_2dp)/100.0);
  lightDataF_out->white = lightData_in->white;
}

void convertSoundDataF(const SoundData_t * soundData_in, SoundData_F_t * soundDataF_out) {
  soundDataF_out->SPL_dBA = ((float) soundData_in->SPL_dBA_int) + 
                           (((float) soundData_in->SPL_dBA_fr_1dp)/10.0);
  for (uint16_t i=0; i<SOUND_FREQ_BANDS; i++) {
    soundDataF_out->SPL_bands_dB[i] = ((float) soundData_in->SPL_bands_dB_int[i]) + 
                                     (((float) soundData_in->SPL_bands_dB_fr_1dp[i])/10.0);
  }
  soundDataF_out->peakAmp_mPa = ((float) soundData_in->peak_amp_mPa_int) + 
                               (((float) soundData_in->peak_amp_mPa_fr_2dp)/100.0);
  soundDataF_out->stable = (soundData_in->stable == 1);
}

void convertParticleDataF(const ParticleData_t * particleData_in, ParticleData_F_t * particleDataF_out) {
  particleDataF_out->duty_cycle_pc = ((float) particleData_in->duty_cycle_pc_int) + 
                                    (((float) particleData_in->duty_cycle_pc_fr_2dp)/100.0);
  particleDataF_out->concentration = ((float) particleData_in->concentration_int) + 
                                    (((float) particleData_in->concentration_fr_2dp)/100.0);
  particleDataF_out->valid = (particleData_in->valid == 1);
}

////////////////////////////////////////////////////////////////////////

// The following five functions print data (in floating-point representation) over the serial port as text

void printAirDataF(const AirData_F_t * airDataF) {
  Serial.print("Temperature = ");Serial.print(airDataF->T_C,2);Serial.println(" C");
  Serial.print("Pressure = ");Serial.print(airDataF->P_Pa);Serial.println(" Pa");
  Serial.print("Humidity = ");Serial.print(airDataF->H_pc,2);Serial.println(" %");
  Serial.print("Gas Sensor Resistance = ");Serial.print(airDataF->G_Ohm);Serial.println(" ohms");
}

void printAirQualityDataF(const AirQualityData_F_t * airQualityDataF) {
  if (airQualityDataF->AQI_accuracy > 0) {
    Serial.print("Air Quality Index = ");Serial.print(airQualityDataF->AQI,2);
    Serial.print(" (");Serial.print(interpret_AQI_value((uint16_t) airQualityDataF->AQI));Serial.println(")");
    Serial.print("Estimated CO2 = ");Serial.print(airQualityDataF->CO2e,2);Serial.println(" ppm");
    Serial.print("Equivalent Breath VOC = ");Serial.print(airQualityDataF->bVOC,2);Serial.println(" ppm");
  }
  Serial.print("Air Quality Accuracy: ");
  Serial.println(interpret_AQI_accuracy(airQualityDataF->AQI_accuracy));
}

void printLightDataF(const LightData_F_t * lightDataF) {
  Serial.print("Illuminance = ");Serial.print(lightDataF->illum_lux,2);Serial.println(" lux");
  Serial.print("White Light Level = ");Serial.print(lightDataF->white);Serial.println();
}

void printSoundDataF(const SoundData_F_t * soundDataF) {
  char strbuf[50] = {0};
  Serial.print("A-weighted Sound Pressure Level = ");
  Serial.print(soundDataF->SPL_dBA,1);Serial.println(" dBA");
  for (uint16_t i=0; i<SOUND_FREQ_BANDS; i++) {
    sprintf(strbuf,"Frequency Band %i (%i Hz) SPL = ", i+1, sound_band_mids_Hz[i]);
    Serial.print(strbuf);
    Serial.print(soundDataF->SPL_bands_dB[i],1);Serial.println(" dB");
  }
  Serial.print("Peak Sound Amplitude = ");Serial.print(soundDataF->peakAmp_mPa,2);Serial.println(" mPa");
}

void printParticleDataF(const ParticleData_F_t * particleDataF, ParticleSensor_t particleSensor) {
  Serial.print("Particle Duty Cycle = ");Serial.print(particleDataF->duty_cycle_pc,2);Serial.println(" %");
  Serial.print("Particle Concentration = ");
  Serial.print(particleDataF->concentration,2);
  if (particleSensor == PPD42) {
    Serial.println(" ppL");
  }
  else if (particleSensor == SDS011) {
    Serial.println(" ug/m3");
  }
  else {
    Serial.println(" (?)");
  }
  Serial.print("Particle data valid: ");
  if (particleDataF->valid) {
    Serial.println("Yes");
  }
  else {
    Serial.println("No (Initializing)");
  }
}

////////////////////////////////////////////////////////////////////////

// The following five functions print data (in integer representation) over the serial port as text.
// printColumns determines the print format:
// choosing printColumns = false gives labeled values with measurement units
// choosing printColumns = true gives columns of numbers (convenient for spreadsheets).

void printAirData(const AirData_t * airData, bool printColumns) {
  char strbuf[50] = {0};
  if (printColumns) {
    // Print: temperature/C, pressure/Pa, humidity/%, gas sensor resistance/ohm 
    uint8_t temp = airData->T_C_int_with_sign & TEMPERATURE_VALUE_MASK;
    if ((airData->T_C_int_with_sign & TEMPERATURE_SIGN_MASK) != 0) {
      // the most-significant bit is set, indicating that the temperature is negative
      sprintf(strbuf,"-%u.%u ",temp,airData->T_C_fr_1dp);
    }
    else {
      sprintf(strbuf,"%u.%u ",temp,airData->T_C_fr_1dp);
    }
    Serial.print(strbuf);
    sprintf(strbuf,"%lu %u.%u %lu ",airData->P_Pa, airData->H_pc_int, airData->H_pc_fr_1dp, airData->G_ohm);
    Serial.print(strbuf);
  }
  else {
    uint8_t temp = airData->T_C_int_with_sign & TEMPERATURE_VALUE_MASK;
    if ((airData->T_C_int_with_sign & TEMPERATURE_SIGN_MASK) != 0) {
      // the most-significant bit is set, indicating that the temperature is negative
      sprintf(strbuf,"Temperature = -%u.%u C",temp,airData->T_C_fr_1dp);
    }
    else {
      // temperature is positive
      sprintf(strbuf,"Temperature = %u.%u C",temp,airData->T_C_fr_1dp);
    }
    Serial.println(strbuf);
    Serial.print("Pressure = ");Serial.print(airData->P_Pa);Serial.println(" Pa");
    sprintf(strbuf,"Humidity = %u.%u %%",airData->H_pc_int,airData->H_pc_fr_1dp);
    Serial.println(strbuf);
    Serial.print("Gas Sensor Resistance = ");Serial.print(airData->G_ohm);Serial.println(" ohm");
  }
}

void printAirQualityData(const AirQualityData_t * airQualityData, bool printColumns) {
  char strbuf[50] = {0};
  if (printColumns) {
    // Print: Air Quality Index, Estimated CO2/ppm, Equivalent breath VOC/ppm, Accuracy
    sprintf(strbuf,"%u.%u %u.%u %u.%02u %u ",airQualityData->AQI_int, airQualityData->AQI_fr_1dp,
            airQualityData->CO2e_int, airQualityData->CO2e_fr_1dp,
            airQualityData->bVOC_int, airQualityData->bVOC_fr_2dp, airQualityData->AQI_accuracy);
    Serial.print(strbuf);
  }
  else {
    if (airQualityData->AQI_accuracy > 0) {
      sprintf(strbuf,"Air Quality Index = %u.%u (%s)",
          airQualityData->AQI_int, airQualityData->AQI_fr_1dp, interpret_AQI_value(airQualityData->AQI_int));
      Serial.println(strbuf);
      sprintf(strbuf,"Estimated CO2 = %u.%u ppm",airQualityData->CO2e_int, airQualityData->CO2e_fr_1dp);
      Serial.println(strbuf);
      sprintf(strbuf,"Equivalent Breath VOC = %u.%02u ppm",
                     airQualityData->bVOC_int, airQualityData->bVOC_fr_2dp);
      Serial.println(strbuf);
    }
    Serial.print("Air Quality Accuracy: ");
    Serial.println(interpret_AQI_accuracy(airQualityData->AQI_accuracy));
  }
}

void printSoundData(const SoundData_t * soundData, bool printColumns) {
  char strbuf[50] = {0};
  if (printColumns) {
    // Print: Sound pressure level/dBA, Sound pressure level for frequency bands 1 to 6 (six columns), 
    //        Peak sound amplitude/mPa, stability 
    sprintf(strbuf,"%u.%u ", soundData->SPL_dBA_int, soundData->SPL_dBA_fr_1dp);
    Serial.print(strbuf);
    for (uint16_t i=0; i<SOUND_FREQ_BANDS; i++) {
      sprintf(strbuf,"%u.%u ", soundData->SPL_bands_dB_int[i], soundData->SPL_bands_dB_fr_1dp[i]);
      Serial.print(strbuf);
    }
    sprintf(strbuf,"%u.%02u %u ", soundData->peak_amp_mPa_int, 
          soundData->peak_amp_mPa_fr_2dp, soundData->stable);
    Serial.print(strbuf);
  }
  else {
    sprintf(strbuf,"A-weighted Sound Pressure Level = %u.%u dBA", 
                   soundData->SPL_dBA_int, soundData->SPL_dBA_fr_1dp);
    Serial.println(strbuf);
    for (uint8_t i=0; i<SOUND_FREQ_BANDS; i++) {
      sprintf(strbuf,"Frequency Band %i (%i Hz) SPL = %u.%u dB", 
            i+1, sound_band_mids_Hz[i], soundData->SPL_bands_dB_int[i], soundData->SPL_bands_dB_fr_1dp[i]);
      Serial.println(strbuf);
    }
    sprintf(strbuf,"Peak Sound Amplitude = %u.%02u mPa", 
                   soundData->peak_amp_mPa_int, soundData->peak_amp_mPa_fr_2dp);
    Serial.println(strbuf);
  }
}

void printLightData(const LightData_t * lightData, bool printColumns) {
  char strbuf[50] = {0};
  if (printColumns) {
    // Print: illuminance/lux, white level
    sprintf(strbuf,"%u.%02u %u ", lightData->illum_lux_int, lightData->illum_lux_fr_2dp, lightData->white);
    Serial.print(strbuf);
  }
  else {
    sprintf(strbuf,"Illuminance = %u.%02u lux", lightData->illum_lux_int, lightData->illum_lux_fr_2dp);
    Serial.println(strbuf);
    Serial.print("White Light Level = ");Serial.print(lightData->white);Serial.println();
  }
}

void printParticleData(const ParticleData_t * particleData, bool printColumns, 
                       ParticleSensor_t particleSensor) {
  char strbuf[50] = {0};
  if (printColumns) {
    // Print: duty cycle/%, concentration
    sprintf(strbuf,"%u.%02u %u.%02u %u ", particleData->duty_cycle_pc_int, 
                   particleData->duty_cycle_pc_fr_2dp, particleData->concentration_int,
                   particleData->concentration_fr_2dp, particleData->valid);
    Serial.print(strbuf);
  }
  else {
    sprintf(strbuf,"Particle Duty Cycle = %u.%02u %%", 
                   particleData->duty_cycle_pc_int, particleData->duty_cycle_pc_fr_2dp);
    Serial.println(strbuf);
    sprintf(strbuf,"Particle Concentration = %u.%02u ", 
                   particleData->concentration_int, particleData->concentration_fr_2dp);
    Serial.print(strbuf);
    if (particleSensor == PPD42) {
      Serial.println(" ppL");
    }
    else if (particleSensor == SDS011) {
      Serial.println(" ug/m3");
    }
    else {
      Serial.println(" (?)");
    }
    Serial.print("Particle data valid: ");
    if (particleData->valid == 0) {
      Serial.println("No (Initializing)");
    }
    else {
      Serial.println("Yes");
    }
  }
}

////////////////////////////////////////////////////////////////////////

// Send data to the Metriful MS430 using the I2C-compatible two wire interface.
//
// Returns true on success, false on failure.
//
// dev_addr_7bit = the 7-bit I2C address of the Metriful board.
// commandRegister = the settings register code or command code to be used.
// data = array containing the data to be sent; its length must be at least "data_length" bytes.
// data_length = the number of bytes from the "data" array to be sent. 
//
bool TransmitI2C(uint8_t dev_addr_7bit, uint8_t commandRegister, uint8_t data[], uint8_t data_length) {

  if (data_length > ARDUINO_WIRE_BUFFER_LIMIT_BYTES) {
    // The Arduino Wire library has a limited internal buffer size
    return false;
  }

  TheWire.beginTransmission(dev_addr_7bit);
  uint8_t bytesWritten = TheWire.write(commandRegister);
  if (data_length > 0) {
    bytesWritten += TheWire.write(data, data_length); 
  }
  if (bytesWritten != (data_length+1)) {
    return false;
  }

  return (TheWire.endTransmission(true) == 0);
}

// Read data from the Metriful MS430 using the I2C-compatible two wire interface.
//
// Returns true on success, false on failure.
//
// dev_addr_7bit = the 7-bit I2C address of the Metriful board.
// commandRegister = the settings register code or data location code to be used.
// data = array to store the received data; its length must be at least "data_length" bytes.
// data_length = the number of bytes to read. 
// 
bool ReceiveI2C(uint8_t dev_addr_7bit, uint8_t commandRegister, uint8_t data[], uint8_t data_length) {

  if (data_length == 0) {
    // Cannot do a zero byte read
    return false;
  }
  
  if (data_length > ARDUINO_WIRE_BUFFER_LIMIT_BYTES) {
    // The Arduino Wire library has a limited internal buffer size
    return false;
  }

  TheWire.beginTransmission(dev_addr_7bit);   
  TheWire.write(commandRegister);  
  if (TheWire.endTransmission(false) != 0) {
    return false;
  }         

  if (TheWire.requestFrom(dev_addr_7bit, data_length, (uint8_t) 1) != data_length) {
    // Did not receive the expected number of bytes
    return false;
  }

  for (uint8_t i=0; i<data_length; i++) {
    if (TheWire.available() > 0) {
      data[i] = TheWire.read();
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////

// Provide a readable interpretation of the accuracy code for 
// the air quality measurements (applies to all air quality data) 
const char * interpret_AQI_accuracy(uint8_t AQI_accuracy_code) {
  switch (AQI_accuracy_code) {
    default:
    case 0:
      return "Not Yet Valid, Self-calibration Incomplete";
    case 1:
      return "Low Accuracy, Self-calibration Ongoing";
    case 2:
      return "Medium Accuracy, Self-calibration Ongoing";
    case 3:
      return "High Accuracy";
  }
}

// Provide a readable interpretation of the AQI (air quality index)  
const char * interpret_AQI_value(uint16_t AQI) {
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
    return "Very Bad";
  }
}

// Set the threshold for triggering a sound interrupt.
//
// Returns true on success, false on failure.
//
// threshold_mPa = peak sound amplitude threshold in milliPascals, any 16-bit integer is allowed.
bool setSoundInterruptThreshold(uint8_t dev_addr_7bit, uint16_t threshold_mPa) {
  uint8_t TXdata[SOUND_INTERRUPT_THRESHOLD_BYTES] = {0};
  TXdata[0] = (uint8_t) (threshold_mPa & 0x00FF);
  TXdata[1] = (uint8_t) (threshold_mPa >> 8);
  return TransmitI2C(dev_addr_7bit, SOUND_INTERRUPT_THRESHOLD_REG, TXdata, SOUND_INTERRUPT_THRESHOLD_BYTES);
}

// Set the threshold for triggering a light interrupt.
//
// Returns true on success, false on failure.
//
// The threshold value in lux units can be fractional and is formed as:
//     threshold = thres_lux_int + (thres_lux_fr_2dp/100)
//
// Threshold values exceeding MAX_LUX_VALUE will be limited to MAX_LUX_VALUE.
bool setLightInterruptThreshold(uint8_t dev_addr_7bit, uint16_t thres_lux_int, uint8_t thres_lux_fr_2dp) {
  uint8_t TXdata[LIGHT_INTERRUPT_THRESHOLD_BYTES] = {0};
  TXdata[0] = (uint8_t) (thres_lux_int & 0x00FF);
  TXdata[1] = (uint8_t) (thres_lux_int >> 8);
  TXdata[2] = thres_lux_fr_2dp;
  return TransmitI2C(dev_addr_7bit, LIGHT_INTERRUPT_THRESHOLD_REG, TXdata, LIGHT_INTERRUPT_THRESHOLD_BYTES);
}
