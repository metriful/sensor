/* 
   interrupts.ino

   Example code for using the Metriful MS430 interrupt outputs. 
   
   Light and sound interrupts are configured and the program then 
   waits forever. When an interrupt occurs, a message prints over
   the serial port, the interrupt is cleared (if set to latch type), 
   and the program returns to waiting. 
   View the output in the Serial Monitor.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// Light level interrupt settings

bool enableLightInterrupts = true;
uint8_t light_int_type = LIGHT_INT_TYPE_LATCH;
// Choose the interrupt polarity: trigger when level rises above 
// threshold (positive), or when level falls below threshold (negative).
uint8_t light_int_polarity = LIGHT_INT_POL_POSITIVE;
uint16_t light_int_thres_lux_i = 100;
uint8_t light_int_thres_lux_f2dp = 50;
// The interrupt threshold value in lux units can be fractional and is formed as:
//     threshold = light_int_thres_lux_i + (light_int_thres_lux_f2dp/100)
// E.g. for a light threshold of 56.12 lux, set:
//     light_int_thres_lux_i = 56
//     light_int_thres_lux_f2dp = 12 


// Sound level interrupt settings

bool enableSoundInterrupts = true;
uint8_t sound_int_type = SOUND_INT_TYPE_LATCH;
uint16_t sound_thres_mPa = 100;

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

uint8_t transmit_buffer[1] = {0};


void setup() {  
  // Initialize the host pins, set up the serial port and reset
  SensorHardwareSetup(I2C_ADDRESS); 
  
  // check that the chosen light threshold is a valid value 
  if (light_int_thres_lux_i > MAX_LUX_VALUE) {
    Serial.println("The chosen light interrupt threshold exceeds the maximum allowed value.");
    while (true) {
      yield();
    }
  }

  if ((!enableSoundInterrupts)&&(!enableLightInterrupts)) {
    Serial.println("No interrupts have been selected.");
    while (true) {
      yield();
    }
  }
  
  if (enableSoundInterrupts) {
    // Set the interrupt type (latch or comparator)
    transmit_buffer[0] = sound_int_type;
    TransmitI2C(I2C_ADDRESS, SOUND_INTERRUPT_TYPE_REG, transmit_buffer, 1);
    
    // Set the threshold
    setSoundInterruptThreshold(I2C_ADDRESS, sound_thres_mPa);
    
    // Enable the interrupt
    transmit_buffer[0] = ENABLED;
    TransmitI2C(I2C_ADDRESS, SOUND_INTERRUPT_ENABLE_REG, transmit_buffer, 1);
  }

  if (enableLightInterrupts) {
    // Set the interrupt type (latch or comparator)
    transmit_buffer[0] = light_int_type;
    TransmitI2C(I2C_ADDRESS, LIGHT_INTERRUPT_TYPE_REG, transmit_buffer, 1);
    
    // Set the threshold
    setLightInterruptThreshold(I2C_ADDRESS, light_int_thres_lux_i, light_int_thres_lux_f2dp);
    
    // Set the interrupt polarity
    transmit_buffer[0] = light_int_polarity;
    TransmitI2C(I2C_ADDRESS, LIGHT_INTERRUPT_POLARITY_REG, transmit_buffer, 1);
    
    // Enable the interrupt
    transmit_buffer[0] = ENABLED;
    TransmitI2C(I2C_ADDRESS, LIGHT_INTERRUPT_ENABLE_REG, transmit_buffer, 1);
  }

  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {
    yield();
  } 

  Serial.println("Waiting for interrupts.");
  Serial.println();
}


void loop() {

  // Check whether a light interrupt has occurred
  if ((digitalRead(L_INT_PIN) == LOW) && enableLightInterrupts) {
    Serial.println("LIGHT INTERRUPT.");
    if (light_int_type == LIGHT_INT_TYPE_LATCH) {
      // Latch type interrupts remain set until cleared by command
      TransmitI2C(I2C_ADDRESS, LIGHT_INTERRUPT_CLR_CMD, 0, 0);
    }
  }
  
  // Check whether a sound interrupt has occurred   
  if ((digitalRead(S_INT_PIN) == LOW) && enableSoundInterrupts) {
    Serial.println("SOUND INTERRUPT.");
    if (sound_int_type == SOUND_INT_TYPE_LATCH) {
      // Latch type interrupts remain set until cleared by command
      TransmitI2C(I2C_ADDRESS, SOUND_INTERRUPT_CLR_CMD, 0, 0);
    }
  }
  
  delay(500);
}
