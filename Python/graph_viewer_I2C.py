"""Real-time display of MS430 data, using a Raspberry Pi.

This example runs on Raspberry Pi only, and the MS430 sensor board
must be connected to the Raspberry Pi I2C/GPIO pins.

An alternate version, "graph_viewer_serial.py" runs on multiple operating
systems (including Windows and Linux) and uses serial over USB to get
data from the MS430 sensor via a microcontroller board (e.g. Arduino,
ESP8266, etc).

This example displays a graphical user interface with real-time
updating graphs showing data from the MS430 sensor board.

Installation instructions for the necessary packages are in the
readme and User Guide.
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

import time
from datetime import datetime
from PyQt5.QtWidgets import QApplication
from GraphViewer import GraphViewer
import Raspberry_Pi.sensor_package.sensor_functions as sensor
import Raspberry_Pi.sensor_package.sensor_constants as const

#########################################################
# USER-EDITABLE SETTINGS

# Choose the time delay between data measurements.
# This can be 3/100/300 seconds in cycle mode, or any
# delay time in on-demand mode.

# Set cycle_period_code=None to use on-demand mode, or choose any
# of: CYCLE_PERIOD_3_S, CYCLE_PERIOD_100_S, CYCLE_PERIOD_300_S
cycle_period_code = None
# OR:
on_demand_delay_ms = 0   # Choose any number of milliseconds
# This delay is in addition to the 0.5 second readout time.

# Temperature and particle data are less accurate if read more
# frequently than every 2 seconds

# Maximum number of values of each variable to store and display:
data_buffer_length = 500

# Specify the particle sensor model (PPD42/SDS011/none) and temperature
# units (Celsuis/Fahrenheit) in Raspberry_Pi/sensor_functions.py

# END OF USER-EDITABLE SETTINGS
#########################################################


class GraphViewerI2C(GraphViewer):
    """Real-time display of MS430 data, using a Raspberry Pi."""

    def __init__(self, buffer_length, cycle_period, OD_delay_ms):
        """Set up the I2C and the MS430 board."""
        super().__init__(buffer_length, sensor.PARTICLE_SENSOR,
                         sensor.USE_FAHRENHEIT)
        self.setupSensorI2C(cycle_period, OD_delay_ms)
        air_quality_data = self.cycle_mode
        particle_data = sensor.PARTICLE_SENSOR != const.PARTICLE_SENSOR_OFF
        flag_data = False
        self.setDataRequired(air_quality_data, particle_data, flag_data)

    def setupSensorI2C(self, cycle_period, OD_delay_ms):
        """Set up the MS430 sensor by selecting mode and particle sensor."""
        (self.GPIO, self.I2C_bus) = sensor.SensorHardwareSetup()
        if sensor.PARTICLE_SENSOR != const.PARTICLE_SENSOR_OFF:
            self.I2C_bus.write_i2c_block_data(
                sensor.i2c_7bit_address,
                const.PARTICLE_SENSOR_SELECT_REG, [sensor.PARTICLE_SENSOR])
        if (cycle_period is None) and (OD_delay_ms is None):
            raise ValueError(
                "Either cycle_period or OD_delay_ms must be specified")
        if cycle_period is not None:  # Cycle mode with 3/100/300 second delays
            self.cycle_mode = True
            self.I2C_bus.write_i2c_block_data(
                sensor.i2c_7bit_address,
                const.CYCLE_TIME_PERIOD_REG, [cycle_period])
            self.I2C_bus.write_byte(
                sensor.i2c_7bit_address, const.CYCLE_MODE_CMD)
        else:  # On-demand mode with chosen time delay between measurements
            self.cycle_mode = False
            self.I2C_bus.write_byte(
                sensor.i2c_7bit_address, const.ON_DEMAND_MEASURE_CMD)
            self.delaying = False
            self.OD_delay_ms = OD_delay_ms

    def getDataFunction(self):
        """Obtain new data from I2C and put in data_buffer.

        Returns True if new data were obtained, else returns False.
        """
        if self.cycle_mode:
            if self.GPIO.event_detected(sensor.READY_pin):
                self.readData()
                return True
        else:  # On-demand mode
            if self.delaying:
                time_now_ms = time.time()*1000
                if (time_now_ms - self.time_start_ms) >= self.OD_delay_ms:
                    # Trigger a new measurement
                    self.I2C_bus.write_byte(
                        sensor.i2c_7bit_address, const.ON_DEMAND_MEASURE_CMD)
                    self.delaying = False
            else:
                if self.GPIO.event_detected(sensor.READY_pin):
                    self.readData()
                    self.delaying = True
                    self.time_start_ms = time.time()*1000
                    return True
        return False

    def readData(self):
        """Read the newly available data from the sensor board."""
        self.setWindowTitle('Indoor Environment Data')
        air_data = sensor.get_air_data(self.I2C_bus)
        air_quality_data = sensor.get_air_quality_data(self.I2C_bus)
        light_data = sensor.get_light_data(self.I2C_bus)
        sound_data = sensor.get_sound_data(self.I2C_bus)
        particle_data = sensor.get_particle_data(
            self.I2C_bus, sensor.PARTICLE_SENSOR)
        self.putDataInBuffer(air_data, air_quality_data,
                             light_data, sound_data, particle_data)

    def appendData(self, start_index, data):
        """Add new data to the data buffer."""
        for i, value in enumerate(data):
            self.data_buffer[start_index + i].append(value)
        return (start_index + len(data))

    def putDataInBuffer(self, air_data, air_quality_data, light_data,
                        sound_data, particle_data):
        """Store the data and also the time/date."""
        i = 0
        i = self.appendData(
                i, [air_data['T'], air_data['P_Pa'],
                    air_data['H_pc'], air_data['G_ohm']])
        if (self.cycle_mode):
            i = self.appendData(i, [air_quality_data['AQI'],
                                    air_quality_data['CO2e'],
                                    air_quality_data['bVOC'],
                                    air_quality_data['AQI_accuracy']])
        i = self.appendData(i, [light_data['illum_lux'], light_data['white']])
        i = self.appendData(i, [sound_data['SPL_dBA']]
                            + [sound_data['SPL_bands_dB'][i] for i in
                            range(0, self.sound_band_number)]
                            + [sound_data['peak_amp_mPa']])
        if sensor.PARTICLE_SENSOR != const.PARTICLE_SENSOR_OFF:
            i = self.appendData(
                    i, [particle_data['duty_cycle_pc'],
                        particle_data['concentration']])
        self.time_data.append(datetime.now().timestamp())


if __name__ == '__main__':
    theApp = QApplication([])
    gv = GraphViewerI2C(data_buffer_length,
                        cycle_period_code, on_demand_delay_ms)
    gv.start()
    theApp.exec_()
