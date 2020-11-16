#  graph_viewer_I2C.py

#  NOTE on operating system/platform:

#  This example runs on Raspberry Pi only, and the MS430 sensor board 
#  must be connected to the Raspberry Pi I2C/GPIO pins.

#  An alternate version, "graph_viewer_serial.py" runs on multiple operating 
#  systems (including Windows and Linux) and uses serial over USB to get 
#  data from the MS430 sensor via a microcontroller board (e.g. Arduino, 
#  ESP8266, etc).

#########################################################

#  This example displays a graphical user interface with real-time 
#  updating graphs showing data from the MS430 sensor board.

#  Installation instructions for the necessary packages are in the 
#  readme and User Guide.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

import datetime
from GraphViewer import *
from Raspberry_Pi.sensor_package.sensor_functions import *

#########################################################
# USER-EDITABLE SETTINGS

# Choose the delay between data measurements. This can be 3/100/300 seconds 
# in cycle mode, or any delay time in on-demand mode

# Set cycle_period_code=None to use on-demand mode
cycle_period_code = None # CYCLE_PERIOD_3_S, CYCLE_PERIOD_100_S, CYCLE_PERIOD_300_S, or None
# OR:
on_demand_delay_ms = 0   # Choose any number of milliseconds
# This delay is in addition to the 0.5 second readout time

# Temperature and particle data are less accurate if read more 
# frequently than every 2 seconds

# Maximum number of values of each variable to store and display:
data_buffer_length = 500

# Specify the particle sensor model (PPD42/SDS011/none) and temperature 
# units (Celsuis/Fahrenheit) in Raspberry_Pi/sensor_functions.py

# END OF USER-EDITABLE SETTINGS
#########################################################

class GraphViewerI2C(GraphViewer):
  def __init__(self, buffer_length, cycle_period, OD_delay_ms):
    super(GraphViewerI2C, self).__init__(buffer_length)
    # Set up the I2C and the MS430 board 
    (self.GPIO, self.I2C_bus) = SensorHardwareSetup()
    if (PARTICLE_SENSOR != PARTICLE_SENSOR_OFF):
      self.I2C_bus.write_i2c_block_data(i2c_7bit_address, PARTICLE_SENSOR_SELECT_REG, [PARTICLE_SENSOR])
      self.get_particle_data = True
      if (PARTICLE_SENSOR == PARTICLE_SENSOR_PPD42):
        self.names_units['Particle concentration'] = self.PPD_unit
      else:
        self.names_units['Particle concentration'] = self.SDS_unit
    else:
      self.get_particle_data = False
    if ((cycle_period is None) and (OD_delay_ms is None)): 
      raise Exception("Either cycle_period or OD_delay_ms must be specified")   
    # Set read mode for the MS430: cycle or on-demand
    if (cycle_period is not None):
      # Use cycle mode with 3/100/300 second delay periods
      self.I2C_bus.write_i2c_block_data(i2c_7bit_address, CYCLE_TIME_PERIOD_REG, [cycle_period])
      self.I2C_bus.write_byte(i2c_7bit_address, CYCLE_MODE_CMD)
      self.cycle_mode = True
    else:
      # Use on-demand mode with any chosen time delay between measurements
      self.I2C_bus.write_byte(i2c_7bit_address, ON_DEMAND_MEASURE_CMD)
      self.delaying = False
      self.OD_delay_ms = OD_delay_ms
      self.cycle_mode = False
    if USE_FAHRENHEIT:
      self.useFahrenheitTemperatureUnits(True)
    # select data variables from name list
    self.indices = list(range(0,4))
    if (self.cycle_mode):
      self.indices += list(range(4,8))
      self.band1_index = 11
    else:
      self.band1_index = 7
    self.indices += list(range(8,18))    
    if (self.get_particle_data):
      self.indices += list(range(19,21))
    self.createDataBuffer()
    self.initializeComboBoxes() 


  # Check for new I2C data
  def getDataFunction(self):
    if (self.cycle_mode):
      if GPIO.event_detected(READY_pin):
        self.readData()
        return True
    else:
      # On-demand mode
      if (self.delaying):
        time_now_ms = (datetime.datetime.now().timestamp())*1000
        if ((time_now_ms-self.time_start_ms) >= self.OD_delay_ms):
          # Trigger a new measurement
          self.I2C_bus.write_byte(i2c_7bit_address, ON_DEMAND_MEASURE_CMD)
          self.delaying = False
      else:
        if GPIO.event_detected(READY_pin):
          self.readData()
          self.delaying = True
          self.time_start_ms = (datetime.datetime.now().timestamp())*1000
          return True
    return False


  def readData(self):
    self.setWindowTitle('Indoor Environment Data')
    air_data = get_air_data(self.I2C_bus)
    air_quality_data = get_air_quality_data(self.I2C_bus)
    light_data = get_light_data(self.I2C_bus)
    sound_data = get_sound_data(self.I2C_bus)
    particle_data = get_particle_data(self.I2C_bus, PARTICLE_SENSOR)
    self.putDataInBuffer(air_data, air_quality_data, light_data, sound_data, particle_data)


  def appendData(self, start_index, data):
    for i,v in enumerate(data):
      self.data_buffer[start_index+i].append(v)
    return (start_index + len(data))


  # Store the data and also the time/date
  def putDataInBuffer(self, air_data, air_quality_data, light_data, sound_data, particle_data):
    i=0
    i = self.appendData(i, [air_data['T'], air_data['P_Pa'], air_data['H_pc'], air_data['G_ohm']])
    if (self.cycle_mode):
      i = self.appendData(i, [air_quality_data['AQI'], air_quality_data['CO2e'], 
                          air_quality_data['bVOC'], air_quality_data['AQI_accuracy']])
    i = self.appendData(i, [light_data['illum_lux'], light_data['white']])
    i = self.appendData(i, [sound_data['SPL_dBA']] + 
              [sound_data['SPL_bands_dB'][i] for i in range(0,self.sound_band_number)] + 
              [sound_data['peak_amp_mPa']])
    if (self.get_particle_data):
      i = self.appendData(i, [particle_data['duty_cycle_pc'], particle_data['concentration']])
    self.time_data.append(datetime.datetime.now().timestamp())



if __name__ == '__main__':
  theApp = QtGui.QApplication([])
  gv = GraphViewerI2C(data_buffer_length, cycle_period_code, on_demand_delay_ms)
  gv.start()
  theApp.exec_()
