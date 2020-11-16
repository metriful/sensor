/* 
   particle_sensor_toggle.ino

   Optional advanced demo. This program shows how to generate an output 
   control signal from one of the host's pins, which can be used to turn 
   the particle sensor on and off. An external transistor circuit is
   also needed - this will gate the sensor power supply according to 
   the control signal. Further details are given in the User Guide.
   
   The program continually measures and displays all environment data
   in a repeating cycle. The user can view the output in the Serial 
   Monitor. After reading the data, the particle sensor is powered off 
   for a chosen number of cycles ("off_cycles"). It is then powered on 
   and read before being powered off again. Sound data are ignored 
   while the particle sensor is on, to avoid its fan noise.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include <Metriful_sensor.h>

//////////////////////////////////////////////////////////
// USER-EDITABLE SETTINGS

// How often to read data; choose only 100 or 300 seconds for this demo
// because the sensor should be on for at least one minute before reading
// its data.
uint8_t cycle_period = CYCLE_PERIOD_100_S;

// How to print the data over the serial port. If printDataAsColumns = true,
// data are columns of numbers, useful for transferring to a spreadsheet
// application. Otherwise, data are printed with explanatory labels and units.
bool printDataAsColumns = false;

// Particle sensor power control options
uint8_t off_cycles = 2;  // leave the sensor off for this many cycles between reads
uint8_t particle_sensor_control_pin = 10; // host pin number which outputs the control signal
bool particle_sensor_ON_state = true; 
// particle_sensor_ON_state is the required polarity of the control 
// signal; true means +V is output to turn the sensor on, while false
// means 0 V is output. Use true for 3.3 V hosts and false for 5 V hosts.

// END OF USER-EDITABLE SETTINGS
//////////////////////////////////////////////////////////

uint8_t transmit_buffer[1] = {0};

// Structs for data
AirData_t airData = {0};
AirQualityData_t airQualityData = {0};
LightData_t lightData = {0};
SoundData_t soundData = {0};
ParticleData_t particleData = {0};

bool particleSensorIsOn = false;
uint8_t particleSensor_count = 0;


void setup() {  
  // Initialize the host pins, set up the serial port and reset:
  SensorHardwareSetup(I2C_ADDRESS); 
  
  // Set up the particle sensor control, and turn it off initially
  pinMode(particle_sensor_control_pin, OUTPUT);
  digitalWrite(particle_sensor_control_pin, !particle_sensor_ON_state);
  particleSensorIsOn = false;
  
  // Apply chosen settings to the MS430
  transmit_buffer[0] = PARTICLE_SENSOR;
  TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, transmit_buffer, 1);
  transmit_buffer[0] = cycle_period;
  TransmitI2C(I2C_ADDRESS, CYCLE_TIME_PERIOD_REG, transmit_buffer, 1);

  // Wait for the serial port to be ready, for displaying the output
  while (!Serial) {
    yield();
  } 

  Serial.println("Entering cycle mode and waiting for data.");
  ready_assertion_event = false;
  TransmitI2C(I2C_ADDRESS, CYCLE_MODE_CMD, 0, 0);
}


void loop() {
  // Wait for the next new data release, indicated by a falling edge on READY
  while (!ready_assertion_event) {
    yield();
  }
  ready_assertion_event = false;

  /* Read data from the MS430 into the data structs. 
  For each category of data (air, sound, etc.) a pointer to the data struct is 
  passed to the ReceiveI2C() function. The received byte sequence fills the data 
  struct in the correct order so that each field within the struct receives
  the value of an environmental quantity (temperature, sound level, etc.)
  */ 
  
  // Air data
  ReceiveI2C(I2C_ADDRESS, AIR_DATA_READ, (uint8_t *) &airData, AIR_DATA_BYTES);
  
  /* Air quality data
  The initial self-calibration of the air quality data may take several
  minutes to complete. During this time the accuracy parameter is zero 
  and the data values are not valid.
  */ 
  ReceiveI2C(I2C_ADDRESS, AIR_QUALITY_DATA_READ, (uint8_t *) &airQualityData, AIR_QUALITY_DATA_BYTES);
  
  // Light data
  ReceiveI2C(I2C_ADDRESS, LIGHT_DATA_READ, (uint8_t *) &lightData, LIGHT_DATA_BYTES);
  
  // Sound data - only read when particle sensor is off
  if (!particleSensorIsOn) {
    ReceiveI2C(I2C_ADDRESS, SOUND_DATA_READ, (uint8_t *) &soundData, SOUND_DATA_BYTES);
  }
  
  /* Particle data
  This requires the connection of a particulate sensor (invalid 
  values will be obtained if this sensor is not present).
  Specify your sensor model (PPD42 or SDS011) in Metriful_sensor.h
  Also note that, due to the low pass filtering used, the 
  particle data become valid after an initial initialization 
  period of approximately one minute.
  */ 
  if (particleSensorIsOn) {
    ReceiveI2C(I2C_ADDRESS, PARTICLE_DATA_READ, (uint8_t *) &particleData, PARTICLE_DATA_BYTES);
  }

  // Print all data to the serial port. The previous loop's particle or
  // sound data will be printed if no reading was done on this loop.
  printAirData(&airData, printDataAsColumns);
  printAirQualityData(&airQualityData, printDataAsColumns);
  printLightData(&lightData, printDataAsColumns);
  printSoundData(&soundData, printDataAsColumns);
  printParticleData(&particleData, printDataAsColumns, PARTICLE_SENSOR);
  Serial.println();
  
  // Turn the particle sensor on/off if required 
  if (particleSensorIsOn) {
    // Stop the particle detection on the MS430
    transmit_buffer[0] = OFF;
    TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, transmit_buffer, 1);
      
    // Turn off the hardware:
    digitalWrite(particle_sensor_control_pin, !particle_sensor_ON_state);
    particleSensorIsOn = false;
  }
  else {
    particleSensor_count++;
    if (particleSensor_count >= off_cycles) {
      // Turn on the hardware:
      digitalWrite(particle_sensor_control_pin, particle_sensor_ON_state);
      
      // Start the particle detection on the MS430
      transmit_buffer[0] = PARTICLE_SENSOR;
      TransmitI2C(I2C_ADDRESS, PARTICLE_SENSOR_SELECT_REG, transmit_buffer, 1);
      
      particleSensor_count = 0;
      particleSensorIsOn = true;
    }   
  }
}
