#  graph_viewer_serial.py

#  NOTE on operating system/platform:

#  This example runs on multiple operating systems (including Windows 
#  and Linux) and uses serial over USB to get data from the MS430 sensor 
#  via a microcontroller board (e.g. Arduino, ESP8266, etc).

#  An alternate version, "graph_viewer_I2C.py" is provided for the 
#  Raspberry Pi, where the MS430 board is directly connected to the Pi
#  using the GPIO/I2C pins.

#########################################################

#  This example displays a graphical user interface with real-time 
#  updating graphs showing data from the MS430 sensor board.

#  Instructions (installation instructions are in the readme / User Guide)

#  1) Program the microcontroller board with either "cycle_readout.ino"
#  or "on_demand_readout.ino", with printDataAsColumns = true

#  2) Connect the microcontroller USB cable to your PC and close any 
#  serial monitor software.

#  3) Put the serial port name (system dependent) in the serial_port_name
#  parameter below.

#  4) Run this program with python3

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

#########################################################
# USER-EDITABLE SETTINGS

# Which particle sensor is connected - this is used to select the 
# displayed measurement units (the microcontroller must also be 
# programmed to use the sensor)
particle_sensor = "PPD42"   # put here: "SDS011", "PPD42", or None

# Choose which temperature label to use for display (NOTE: the actual 
# measurement unit depends on the microcontroller program).
# Celsius is default.
use_fahrenheit = False

# Specify the serial port name on which the microcontroller is connected
# e.g. on Windows this is usually a name like "COM1", on Linux it is
# usually a path like "/dev/ttyACM0"
# NOTE: close all other serial applications (e.g. Arduino Serial Monitor)
serial_port_name = "/dev/ttyACM0"

# Maximum number of values of each variable to store and display:
data_buffer_length = 500

# END OF USER-EDITABLE SETTINGS
#########################################################

import datetime
import serial
from GraphViewer import *

class GraphViewerSerial(GraphViewer):
  def __init__(self, buffer_length, serial_port):
    super(GraphViewerSerial, self).__init__(buffer_length)
    self.serial_port = serial.Serial(
      port = serial_port,
      baudrate = 9600,
      parity=serial.PARITY_NONE,
      stopbits=serial.STOPBITS_ONE,
      bytesize=serial.EIGHTBITS,
      timeout=0.02)
    self.startup = True
    self.initial_discard_lines = 2
    self.line_count = 0
    # There are 4 input cases resulting from the use of cycle_readout.ino and
    # on_demand_readout.ino, with 15, 18, 19 and 22 data columns. These lists
    # define which variables are present in each case:
    self.col_indices = []
    self.col_indices.append(list(range(0,4)) + list(range(8,19))) # no air quality or particle data
    self.col_indices.append(list(range(0,4)) + list(range(8,22))) # no air quality data
    self.col_indices.append(list(range(0,19)))                    # no particle data
    self.col_indices.append(list(range(0,22)))                    # all data
    self.sound_band1_index = [7, 7, 11, 11]  # the index of 'Band 1 SPL' in the four lists


  # Allow for initial corrupted serial data, incomplete lines or printed 
  # messages by discarding lines until a correct number of columns appears
  def serialStartupCompleted(self, data_strings):
    if (self.startup):
      self.line_count+=1
      if (self.line_count >= self.initial_discard_lines):
        nc = len(data_strings)
        for i in range(0,len(self.col_indices)):
          if (nc == len(self.col_indices[i])):
            self.startup = False
            self.indices = self.col_indices[i]
            self.band1_index = self.sound_band1_index[i]
            self.createDataBuffer()
            self.initializeComboBoxes()
            self.setWindowTitle('Indoor Environment Data')
        if (self.startup):
          raise Exception('Unexpected number of data columns')
    return (not self.startup)


  # Check for new serial data
  def getDataFunction(self):
    response = self.serial_port.readline()
    if (not ((response is None) or (len(response) == 0))):
      # A complete line was received: convert it to string and split at spaces:
      try:
        data_strings = response.decode('utf-8').split()
        if (self.serialStartupCompleted(data_strings)):
          # Check number of values received; if incorrect, ignore the data
          if (len(data_strings) == len(self.indices)):
            # Convert strings to numbers and store the data
            float_data = [float(i) for i in data_strings]
            for i in range(0, len(self.indices)):
              self.data_buffer[i].append(float_data[i])
            self.time_data.append(datetime.datetime.now().timestamp())
            return True
      except:
        pass
    return False # no new data



if __name__ == '__main__':
  theApp = QtGui.QApplication([])
  gv = GraphViewerSerial(data_buffer_length, serial_port_name)
  gv.setParticleUnits(particle_sensor)
  gv.useFahrenheitTemperatureUnits(use_fahrenheit)
  gv.start()
  theApp.exec_()
