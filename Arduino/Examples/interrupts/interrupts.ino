/* 
   interrupts.ino

   Example code for using the Sense board interrupts. 
   
   Light and sound interrupts are configured and the program then 
   waits indefinitely. When an interrupt occurs, a message is 
   displayed and the interrupt is cleared (if set to latch type) 
   and the program returns to waiting. View the output in the 
   Serial Monitor.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit https://github.com/metriful/sense
*/

#include <Metriful_Sense.h>
#include <stdint.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// Light level interrupt settings.
// Interrupts will cause a message to be printed over the serial port.
bool enableLightInterrupts = true;
uint8_t light_int_type = LIGHT_INT_TYPE_LATCH;
// Choose the interrupt polarity: trigger on level rising above threshold (positive), or
// on level falling below threshold (negative).
uint8_t light_int_polarity = LIGHT_INT_POL_POSITIVE;
uint16_t light_int_thres_lux_i = 100;
uint8_t light_int_thres_lux_f2dp = 50;
// "light_int_thres_lux_f2dp" is the fractional part of the light
// threshold, to two decimal places, multiplied by 100.
// "light_int_thres_lux_i" is the integer part.
// Thus, for a light threshold of 56.12 lux, set:
//     light_int_thres_lux_i = 56
//     light_int_thres_lux_f2dp = 12 
  
// Sound level interrupt settings.
// Interrupts will cause a message to be printed over the serial port.
bool enableSoundInterrupts = true;
uint8_t sound_int_type = SOUND_INT_TYPE_LATCH;
uint16_t sound_thres_mPa = 100;

// The I2C address of the Sense board
uint8_t i2c_7bit_address = I2C_ADDR_7BIT_SB_OPEN;

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

// Ensure tx/rx buffers are big enough to fit the largest send/receive transactions:
uint8_t transmit_buffer[LIGHT_INTERRUPT_THRESHOLD_BYTES] = {0};
uint8_t receive_buffer[SOUND_DATA_BYTES] = {0};


void setup() {  
  // Initialize the Arduino pins, set up the serial port and reset:
  SenseHardwareSetup(i2c_7bit_address); 
  
  // check that the chosen light threshold is a valid value 
  if (light_int_thres_lux_i > MAX_LUX_VALUE) {
    Serial.println("The chosen light interrupt threshold exceeds the maximum allowed value.");
    while (true) {}
  }

  if ((!enableSoundInterrupts)&&(!enableLightInterrupts)) {
    Serial.println("No interrupts have been enabled.");
    while (true) {}
  }
  
  if (enableSoundInterrupts) {
    // Set the interrupt type (latch or comparator)
    transmit_buffer[0] = sound_int_type;
    TransmitI2C(i2c_7bit_address, SOUND_INTERRUPT_TYPE_REG, transmit_buffer, 1);
    // The 16-bit threshold value is split and sent as two 8-bit values:
    transmit_buffer[0] = sound_thres_mPa & 0x00FF;
    transmit_buffer[1] = sound_thres_mPa >> 8;
    TransmitI2C(i2c_7bit_address, SOUND_INTERRUPT_THRESHOLD_REG, transmit_buffer, 
                SOUND_INTERRUPT_THRESHOLD_BYTES);
    // Enable the interrupt on the Sense board:
    transmit_buffer[0] = ENABLED;
    TransmitI2C(i2c_7bit_address, SOUND_INTERRUPT_ENABLE_REG, transmit_buffer, 1);
  }

  if (enableLightInterrupts) {
    // Set the interrupt type (latch or comparator)
    transmit_buffer[0] = light_int_type;
    TransmitI2C(i2c_7bit_address, LIGHT_INTERRUPT_TYPE_REG, transmit_buffer, 1);
    // The 16-bit integer part of the threshold value is split and sent as two 8-bit values,
    // while the fractional part is sent as an 8-bit value:
    transmit_buffer[0] = light_int_thres_lux_i & 0x00FF;
    transmit_buffer[1] = light_int_thres_lux_i >> 8;
    transmit_buffer[2] = light_int_thres_lux_f2dp;
    TransmitI2C(i2c_7bit_address, LIGHT_INTERRUPT_THRESHOLD_REG, transmit_buffer, 
                LIGHT_INTERRUPT_THRESHOLD_BYTES);
    // Set the interrupt polarity
    transmit_buffer[0] = light_int_polarity;
    TransmitI2C(i2c_7bit_address, LIGHT_INTERRUPT_POLARITY_REG, transmit_buffer, 1);
    // Enable the interrupt on the Sense board:
    transmit_buffer[0] = ENABLED;
    TransmitI2C(i2c_7bit_address, LIGHT_INTERRUPT_ENABLE_REG, transmit_buffer, 1);
  }

  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {} 

  Serial.println("Waiting for interrupts.");
  Serial.println();
}


void loop() {

  // Check whether a light interrupt has occurred
  if ((digitalRead(L_INT_PIN) == LOW) && enableLightInterrupts) {
        Serial.println("LIGHT INTERRUPT.");
    if (light_int_type == LIGHT_INT_TYPE_LATCH) {
      // Latch type interrupts remain set until cleared by command
      TransmitI2C(i2c_7bit_address, LIGHT_INTERRUPT_CLR_CMD, transmit_buffer, 0);
    }
  }
  
  // Check whether a sound interrupt has occurred   
  if ((digitalRead(S_INT_PIN) == LOW) && enableSoundInterrupts) {
    Serial.println("SOUND INTERRUPT.");
    if (sound_int_type == SOUND_INT_TYPE_LATCH) {
      // Latch type interrupts remain set until cleared by command
      TransmitI2C(i2c_7bit_address, SOUND_INTERRUPT_CLR_CMD, transmit_buffer, 0);
    }
  }
  
  delay(500);
}
