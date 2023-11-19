"""Real-time graphical display of MS430 data.

This file defines a class for creating a graphical user interface,
to display graphs with real-time data updates.

A subclass must be derived from GraphViewer to implement the
method getDataFunction() and create a working example program. This
is done in "graph_viewer_serial.py" and "graph_viewer_I2C.py"

This is designed to run with Python 3 on multiple operating systems.
The readme and User Guide give instructions on installing the necessary
packages (pyqtgraph and PyQt5).
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

#########################################################

from PyQt5 import QtCore
from PyQt5.QtWidgets import QMainWindow, QWidget, QGridLayout
import pyqtgraph as pg
from collections import deque
import Raspberry_Pi.sensor_package.sensor_constants as const


class GraphViewer(QMainWindow):
    """Real-time graphical display of MS430 data."""

    def __init__(self, data_buffer_length,
                 particle_sensor_type, use_fahrenheit):
        """Set up a grid layout of data graphs.

        data_buffer_length: number of data points stored/displayed
        """
        super().__init__()

        self.buffer_samples = data_buffer_length

        # graphs_vertical, graphs_horizontal: grid size
        self.graphs_vertical = 2
        self.graphs_horizontal = 2

        # Time delay (milliseconds) between graph updates
        self.update_time_period_ms = 20

        # Appearance settings:
        self.pen_style = pg.mkPen(color="y", width=2,
                                  style=QtCore.Qt.PenStyle.SolidLine)
        self.title_color = "w"
        self.title_size = "13pt"
        self.axis_color = "w"
        self.axis_label_style = {'color': 'w', 'font-size': '11pt'}
        self.x_grid = False
        self.y_grid = False
        self.data_names_units = {'Temperature': '',
                                 'Pressure': 'Pa', 'Humidity': '%',
                                 'Gas sensor resistance': "\u03A9",
                                 'Air Quality Index': '',
                                 'Estimated CO\u2082': 'ppm',
                                 'Equivalent breath VOC': 'ppm',
                                 'Air quality accuracy': '',
                                 'Illuminance': 'lux',
                                 'White light level': '',
                                 'Sound pressure level': 'dBA',
                                 'Band 1 SPL': 'dB',
                                 'Band 2 SPL': 'dB', 'Band 3 SPL': 'dB',
                                 'Band 4 SPL': 'dB', 'Band 5 SPL': 'dB',
                                 'Band 6 SPL': 'dB',
                                 'Peak sound amplitude': 'mPa',
                                 'Microphone initialized': '',
                                 'Particle sensor duty cycle': '%',
                                 'Particle concentration': '',
                                 'Particle data valid': ''}
        self.sound_band_name = 'Sound frequency bands'
        self.setParticleUnits(particle_sensor_type)
        self.setTemperatureUnits(use_fahrenheit)
        self.decimal_places = [1, 0, 1, 0, 1, 1, 2, 0,
                               2, 0, 1, 1, 1, 1, 1, 1, 1, 2, 0, 2, 2, 0]
        self.sound_band_number = len(const.sound_band_mids_Hz)
        # displayed_combo_index = the combobox index of the variable
        #                         currently displayed on each graph.
        # selected_combo_index = the combobox index of the variable
        #                        selected by each combobox menu.
        self.displayed_combo_index = []
        self.selected_combo_index = []
        self.time_data = deque(maxlen=self.buffer_samples)
        self.setWindowTitle('Waiting for data...')
        self.createUI()

    def createUI(self):
        """Construct the user interface."""
        self.widget = QWidget()
        self.setCentralWidget(self.widget)
        self.widget.setLayout(QGridLayout())
        self.plot_items = []
        self.plot_handles = []
        self.combos = []
        self.is_bar_chart = []
        for nv in range(self.graphs_vertical):
            for nh in range(self.graphs_horizontal):
                GLW = pg.GraphicsLayoutWidget()
                combo = pg.ComboBox()
                self.combos.append(combo)
                self.widget.layout().addWidget(combo, (2*nv), nh)
                self.widget.layout().addWidget(GLW, (2*nv)+1, nh)
                new_plot = GLW.addPlot()
                self.plot_items.append(new_plot)
                self.plot_handles.append(
                    new_plot.plot(pen=self.pen_style, symbol=None,
                                  axisItems={'bottom': pg.DateAxisItem()}))
                self.formatPlotItem(new_plot)
                self.is_bar_chart.append(False)

    def setDataRequired(self, air_quality_data, particle_data, flag_data):
        """Indicate which variables from the name list will be available."""
        self.data_name_index = list(range(0, 4))
        if air_quality_data:
            self.data_name_index += list(range(4, 8))
            self.band1_index = 11
        else:
            self.band1_index = 7
        self.data_name_index += list(range(8, 18))
        if flag_data:
            self.data_name_index.append(18)
        if particle_data:
            self.data_name_index += list(range(19, 21))
        if flag_data:
            self.data_name_index.append(21)
        self.createDataBuffer()
        self.initializeComboBoxes()

    def start(self):
        """Begin the periodic updating of the GUI."""
        self.updateLoop()
        self.show()

    def setParticleUnits(self, particle_sensor_type):
        """Set the particulate unit, depending on hardware (if any)."""
        if particle_sensor_type == const.PARTICLE_SENSOR_SDS011:
            self.data_names_units['Particle concentration'] = const.SDS011_CONC_SYMBOL
        elif particle_sensor_type == const.PARTICLE_SENSOR_PPD42:
            self.data_names_units['Particle concentration'] = "ppL"
        elif particle_sensor_type != const.PARTICLE_SENSOR_OFF:
            raise ValueError("Particle sensor type not recognized")

    def setTemperatureUnits(self, use_fahrenheit):
        """Set either C or F for temperature display."""
        if use_fahrenheit:
            self.data_names_units['Temperature'] = const.FAHRENHEIT_SYMBOL
        else:
            self.data_names_units['Temperature'] = const.CELSIUS_SYMBOL

    def formatPlotItem(self, item):
        """Adjust plot appearance."""
        item.setMenuEnabled(False)
        item.showGrid(x=self.x_grid, y=self.y_grid)
        item.getAxis("left").setPen(pg.mkPen(self.axis_color))
        item.getAxis("bottom").setPen(pg.mkPen(self.axis_color))
        item.getAxis("left").setStyle(tickLength=7)
        item.getAxis("bottom").setStyle(tickLength=7)
        item.setAxisItems({'bottom': pg.DateAxisItem()})

    def funcCreator(self, graph_index, combo_handle):
        """Create functions to be executed on combobox change action."""
        def func():
            self.selected_combo_index[graph_index] = combo_handle.value()
        return func

    def updateLoop(self):
        """Check for new data and redraw the graphs if anything changed."""
        need_update = self.displayed_combo_index != self.selected_combo_index
        need_update = need_update or self.getDataFunction()
        if (need_update):
            self.updateGraphs()
        QtCore.QTimer.singleShot(self.update_time_period_ms, self.updateLoop)

    def getDataFunction(self):
        """Obtain new data (hardware-dependent) and put in data_buffer.

        Returns True if new data were obtained, else returns False.
        """
        raise NotImplementedError("Override this method in a derived class")

    def createDataBuffer(self):
        """Store data for each graph in a deque."""
        self.data_buffer = [deque(maxlen=self.buffer_samples)
                            for i in range(len(self.data_name_index))]

    def initializeComboBoxes(self):
        """Fill the ComboBoxes and set the initial selected values."""
        names = [list(self.data_names_units.keys())[j]
                 for j in self.data_name_index]
        combo_items = dict(zip(names, [k for k in range(len(names))]))
        combo_items[self.sound_band_name] = len(combo_items)
        for n, combo in enumerate(self.combos):
            combo.setItems(combo_items)
            start_index = n
            if (n == 0):  # Set first plot to be a bar chart
                start_index = combo_items[self.sound_band_name]
            self.selected_combo_index.append(start_index)
            combo.setValue(start_index)
            combo.currentIndexChanged.connect(self.funcCreator(n, combo))
        self.displayed_combo_index = self.selected_combo_index.copy()

    def updateGraphs(self):
        """Draw new data on the graphs and update the text label titles."""
        for n in range(len(self.plot_handles)):
            self.displayed_combo_index[n] = self.selected_combo_index[n]
            bar_chart = (self.displayed_combo_index[n]
                         >= len(self.data_name_index))
            if bar_chart != self.is_bar_chart[n]:
                # Chart type has just changed: initialize new type:
                self.changeChartType(n, bar_chart)
            if bar_chart:
                new_data = [self.data_buffer[i][-1] for i in range(
                            self.band1_index,
                            self.band1_index + self.sound_band_number)]
                self.plot_handles[n].setOpts(height=new_data)
            else:  # Line graph of single variable
                ind = self.data_name_index[self.displayed_combo_index[n]]
                self.plot_items[n].setTitle(
                    list(self.data_names_units.keys())[ind]
                    + " = {:.{dps}f} ".format(
                        self.data_buffer[self.displayed_combo_index[n]][-1],
                        dps=self.decimal_places[ind])
                    + list(self.data_names_units.values())[ind],
                    color=self.title_color, size=self.title_size)
                self.plot_handles[n].setData(
                    self.time_data,
                    self.data_buffer[self.displayed_combo_index[n]])

    def changeChartType(self, plot_index, is_bar_chart):
        """Switch between bar chart (for sound frequencies) and line graph."""
        self.plot_items[plot_index].removeItem(self.plot_handles[plot_index])
        self.plot_handles[plot_index].deleteLater()
        self.is_bar_chart[plot_index] = is_bar_chart
        if is_bar_chart:
            self.plot_handles[plot_index] = pg.BarGraphItem(
                x=list(range(self.sound_band_number)),
                height=[0]*self.sound_band_number,
                width=0.9, brush="r")
            self.plot_items[plot_index].addItem(self.plot_handles[plot_index])
            self.formatBarChart(self.plot_items[plot_index])
        else:  # Line graph of single variable
            self.plot_handles[plot_index] = self.plot_items[plot_index].plot(
                pen=self.pen_style, symbol=None)
            self.adjustAxes(self.plot_items[plot_index])

    def adjustAxes(self, item):
        """Format the line graph axis settings."""
        item.getAxis("bottom").setTicks(None)
        item.getAxis("left").setTicks(None)
        item.getAxis("bottom").showLabel(False)
        item.enableAutoRange(axis='x', enable=True)
        item.enableAutoRange(axis='y', enable=True)

    def formatBarChart(self, item):
        """Format the bar chart for the sound frequency bands."""
        dB_labels = [20, 30, 40, 50, 60, 70, 80, 90]
        item.setTitle("Frequency band sound level / dB",
                      color=self.title_color, size=self.title_size)
        item.setLabel('bottom', text="Band center frequency / Hz",
                      **self.axis_label_style)
        # X axis ticks: set minor to same as major
        # and label according to frequency
        x_ticks = [[], []]
        for n, x in enumerate(const.sound_band_mids_Hz):
            x_ticks[0].append((n, str(x)))
            x_ticks[1].append(x_ticks[0][n])
        item.getAxis("bottom").setTicks(x_ticks)
        item.setXRange(-0.5, 5.5, padding=0)
        # Y axis ticks: set minor ticks to same as major
        y_ticks = [[], []]
        for n, y in enumerate(dB_labels):
            y_ticks[0].append((y, str(y)))
            y_ticks[1].append(y_ticks[0][n])
        item.getAxis("left").setTicks(y_ticks)
        item.setYRange(dB_labels[0], dB_labels[-1], padding=0.05)
