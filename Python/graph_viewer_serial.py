"""Real-time display of MS430 data, from a host device over USB serial.

This example runs on multiple operating systems (including Windows
and Linux) and uses serial over USB to get data from the MS430 sensor
via a microcontroller board (e.g. Arduino, ESP8266, etc).

An alternate version, "graph_viewer_I2C.py" is provided for the
Raspberry Pi, where the MS430 board is directly connected to the Pi
using the GPIO/I2C pins.

This example displays a graphical user interface with real-time
updating graphs showing data from the MS430 sensor board.

Instructions (installation instructions are in the readme / User Guide)

1) Program the microcontroller board with either "cycle_readout.ino"
or "on_demand_readout.ino", with printDataAsColumns = true

2) Connect the microcontroller USB cable to your PC and close any
serial monitor software.

3) Put the serial port name (system dependent) in the serial_port_name
parameter below.

4) Run this program with python3
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

import serial
from datetime import datetime
from PyQt5.QtWidgets import QApplication
from GraphViewer import GraphViewer
import Raspberry_Pi.sensor_package.sensor_constants as const

#########################################################
# USER-EDITABLE SETTINGS

# Specify the serial port name on which the microcontroller is connected
# e.g. on Windows this is usually a name like "COM1", on Linux it is
# usually a path like "/dev/ttyACM0"
# NOTE: close all other serial applications (e.g. Arduino Serial Monitor)
serial_port_name = "/dev/ttyACM0"

# Maximum number of values of each variable to store and display:
data_buffer_length = 500

# Specify the particle sensor model (PPD42/SDS011/none) and temperature
# units (Celsius/Fahrenheit):
particle_sensor_type = const.PARTICLE_SENSOR_OFF
use_fahrenheit = False  # else uses Celsius

# END OF USER-EDITABLE SETTINGS
#########################################################


class GraphViewerSerial(GraphViewer):
    """Real-time display of MS430 data, from a host device over USB serial."""

    def __init__(self, buffer_length, serial_port):
        """Set up the serial interface to the MS430 host."""
        super().__init__(buffer_length, particle_sensor_type, use_fahrenheit)
        self.serial_port = serial.Serial(
            port=serial_port,
            baudrate=9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=0.02)
        self.initial_discard_lines = 2
        self.startup_line_count = 0
        self.startup = True

    def serialStartupCompleted(self, data_strings):
        """Check that the number of values received is correct."""
        if not self.startup:
            return True
        if self.startup_line_count < self.initial_discard_lines:
            self.startup_line_count += 1
            return False
        self.startup = False
        if len(data_strings) == 15:
            self.setDataRequired(False, False, True)
        elif len(data_strings) == 18:
            self.setDataRequired(False, True, True)
        elif len(data_strings) == 19:
            self.setDataRequired(True, False, True)
        elif len(data_strings) == 22:
            self.setDataRequired(True, True, True)
        else:
            raise RuntimeError('Unexpected number of data columns')
        self.setWindowTitle('Indoor Environment Data')
        return True

    def getDataFunction(self):
        """Check for new serial data."""
        response = self.serial_port.readline()
        if (not ((response is None) or (len(response) == 0))):
            try:
                data_strings = response.decode('utf-8').split()
                if self.serialStartupCompleted(data_strings):
                    if (len(data_strings) == len(self.data_name_index)):
                        # Convert strings to numbers and store the data
                        float_data = [float(s) for s in data_strings]
                        for i in range(len(self.data_name_index)):
                            self.data_buffer[i].append(float_data[i])
                        self.time_data.append(datetime.now().timestamp())
                        return True
            except Exception:
                pass
        return False  # no new data


if __name__ == '__main__':
    theApp = QApplication([])
    gv = GraphViewerSerial(data_buffer_length, serial_port_name)
    gv.start()
    theApp.exec_()
