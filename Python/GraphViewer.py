#  GraphViewer.py

#  This file defines a class for creating a graphical user interface, 
#  to display graphs with real-time data updates.

#  A subclass must be derived from GraphViewer to implement the 
#  method getDataFunction() and create a working example program. This
#  is done in "graph_viewer_serial.py" and "graph_viewer_I2C.py"

#  This is designed to run with Python 3 on multiple operating systems. 
#  The readme and User Guide give instructions on installing the necessary 
#  packages (pyqtgraph and PyQt5).

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

#########################################################

import time
from pyqtgraph.Qt import QtCore, QtGui
import pyqtgraph as pg
from collections import deque, OrderedDict


class GraphViewer(QtGui.QMainWindow):
  def __init__(self, data_buffer_length):
    super(GraphViewer, self).__init__()
    
    self.buffer_samples = data_buffer_length 
    
    # Define the number of graphs in the grid:
    graphs_vertical = 2
    graphs_horizontal = 2
    
    # Appearance settings:
    self.pen_style = pg.mkPen(color="y", width=2, style=QtCore.Qt.SolidLine)
    self.title_color = "w"
    self.title_size = "13pt"
    self.axis_color = "w"
    self.axis_label_style = {'color': '#FFF', 'font-size': '11pt'}
    self.x_grid = False
    self.y_grid = False
    
    # Labels and measurement units for display
    self.C_label = "\u00B0C"
    self.F_label = "\u00B0F"
    self.SDS_unit = "\u00B5g/m\u00B3"
    self.PPD_unit = "ppL"
    self.names_units = OrderedDict([('Temperature', self.C_label),
      ('Pressure', 'Pa'),('Humidity', '%'),('Gas sensor resistance', "\u03A9"),
      ('Air Quality Index', ''),('Estimated CO\u2082', 'ppm'),('Equivalent breath VOC', 'ppm'),
      ('Air quality accuracy', ''),('Illuminance', 'lux'),('White light level', ''),
      ('Sound pressure level', 'dBA'),('Band 1 SPL', 'dB'),('Band 2 SPL', 'dB'),
      ('Band 3 SPL', 'dB'),('Band 4 SPL', 'dB'),('Band 5 SPL', 'dB'),
      ('Band 6 SPL', 'dB'),('Peak sound amplitude', 'mPa'),('Microphone initialized', ''),
      ('Particle sensor duty cycle', '%'),('Particle concentration', self.PPD_unit),
      ('Particle data valid', '')])
    self.decimal_places = [1,0,1,0,1,1,2,0,2,0,1,1,1,1,1,1,1,2,0,2,2,0]
    self.sound_band_number = 6
  
    # Construct the user interface
    self.setWindowTitle('Waiting for data...')
    self.widget = QtGui.QWidget()
    self.setCentralWidget(self.widget)
    self.widget.setLayout(QtGui.QGridLayout())
    self.graph_var_numbers = [] 
    self.selected_var_numbers = []
    self.plot_items = []
    self.plot_handles = []
    self.combos = []
    self.is_bar_chart = []
    for nv in range(0,graphs_vertical):
      for nh in range(0,graphs_horizontal):
        GLW = pg.GraphicsLayoutWidget()
        combo = pg.ComboBox()
        self.combos.append(combo)
        self.widget.layout().addWidget(combo, (2*nv), nh) 
        self.widget.layout().addWidget(GLW, (2*nv)+1, nh) 
        new_plot = GLW.addPlot()
        self.plot_items.append(new_plot)
        self.plot_handles.append(new_plot.plot(pen=self.pen_style, 
          symbol=None, axisItems={'bottom': pg.DateAxisItem()}))
        self.formatPlotItem(new_plot)
        self.is_bar_chart.append(False)
    self.time_data = deque(maxlen=self.buffer_samples)


  # Initialize and begin the periodic updating of the GUI
  def start(self):
    self.updateLoop()
    self.show()


  def setParticleUnits(self, name):
    if (name == "SDS011"):
      self.names_units['Particle concentration'] = self.SDS_unit
    elif (name == "PPD42"):
      self.names_units['Particle concentration'] = self.PPD_unit
    elif (name is not None):
      raise Exception("Particle sensor name must be 'SDS011' or 'PPD42', or None")


  def useFahrenheitTemperatureUnits(self, use_fahrenheit):
    if (use_fahrenheit):
      self.names_units['Temperature'] = self.F_label
    else:
      self.names_units['Temperature'] = self.C_label
      

  # Adjust plot appearance
  def formatPlotItem(self, item):
    item.setMenuEnabled(False)
    item.showGrid(x=self.x_grid, y=self.y_grid)
    item.getAxis("left").setPen(pg.mkPen(self.axis_color))
    item.getAxis("bottom").setPen(pg.mkPen(self.axis_color))
    item.getAxis("left").setStyle(tickLength=7)
    item.getAxis("bottom").setStyle(tickLength=7)
    item.setAxisItems({'bottom':pg.DateAxisItem()})


  # Create and return a new function which will be called when one of
  # the comboboxes is changed.
  def funcCreator(self, graph_index, combo_handle):
    def func():
      self.selected_var_numbers[graph_index] = combo_handle.value()
    return func


  # Check for new data and redraw the graphs if data or combobox 
  # selections have changed
  def updateLoop(self):
    need_update = (self.graph_var_numbers != self.selected_var_numbers)
    need_update = need_update or self.getDataFunction()
    if (need_update):
      self.updateGraphs()
    # Call this function again in 20 ms
    QtCore.QTimer.singleShot(20, self.updateLoop)


  def getDataFunction(self):
    # To be defined in subclass
    pass


  def createDataBuffer(self):
    self.data_buffer = [deque(maxlen=self.buffer_samples) for i in range(0, len(self.indices))]


  # Fill the ComboBoxes with the list of items and set the initial selected values 
  def initializeComboBoxes(self):
    names = [list(self.names_units.keys())[j] for j in self.indices]
    combo_items = dict(zip(names, [k for k in range(0,len(names))]))
    combo_items['Sound frequency bands'] = len(combo_items)
    for n,combo in enumerate(self.combos):
      combo.setItems(combo_items)
      start_index = n
      if (n==0):
        # Set first plot to be a bar chart
        start_index = combo_items['Sound frequency bands']
      self.selected_var_numbers.append(start_index)
      combo.setValue(start_index)
      combo.currentIndexChanged.connect(self.funcCreator(n, combo))
    self.graph_var_numbers = self.selected_var_numbers.copy()


  # Draw new data on the graphs and update the text label titles
  def updateGraphs(self):
    for n in range(0,len(self.plot_handles)):
      self.graph_var_numbers[n] = self.selected_var_numbers[n]
      if (self.graph_var_numbers[n] >= len(self.indices)):
        # Bar chart of sound bands
        if not (self.is_bar_chart[n] == True):
          self.plot_items[n].removeItem(self.plot_handles[n])
          self.plot_handles[n].deleteLater()
          self.plot_handles[n] = pg.BarGraphItem(x=list(range(0,self.sound_band_number)), 
                                height=[0]*self.sound_band_number, width=0.9, brush="r")
          self.plot_items[n].addItem(self.plot_handles[n])
          self.formatBarChart(self.plot_items[n])
          self.is_bar_chart[n] = True
        new_data = [self.data_buffer[i][-1] for i in range(self.band1_index,
                   self.band1_index+self.sound_band_number)]
        self.plot_handles[n].setOpts(height=new_data)
      else:
        # Line graph of single variable
        if not (self.is_bar_chart[n] == False):
          self.plot_items[n].removeItem(self.plot_handles[n])
          self.plot_handles[n].deleteLater()
          self.plot_handles[n] = self.plot_items[n].plot(pen=self.pen_style, symbol=None)
          self.adjustAxes(self.plot_items[n])
          self.is_bar_chart[n] = False
        ind = self.indices[self.graph_var_numbers[n]] 
        self.plot_items[n].setTitle(list(self.names_units.keys())[ind] + 
          " = {:.{dps}f} ".format(self.data_buffer[self.graph_var_numbers[n]][-1],
          dps=self.decimal_places[ind]) + list(self.names_units.values())[ind],
          color=self.title_color,size=self.title_size)
        self.plot_handles[n].setData(self.time_data, self.data_buffer[self.graph_var_numbers[n]])


  # Change axis settings
  def adjustAxes(self, item):
    item.getAxis("bottom").setTicks(None)
    item.getAxis("left").setTicks(None)
    item.getAxis("bottom").showLabel(False)
    item.enableAutoRange(axis='x', enable=True)  
    item.enableAutoRange(axis='y', enable=True)  


  # Format the bar chart for displaying sound data for the six frequency bands
  def formatBarChart(self, item):
    frequency_midpoints = [125, 250, 500, 1000, 2000, 4000]
    dB_labels = [20,30,40,50,60,70,80,90]
    item.setTitle("Frequency band sound level / dB",color=self.title_color,size=self.title_size)
    item.setLabel('bottom', text="Band center frequency / Hz", **self.axis_label_style)
    # X axis ticks: set minor to same as major and label according to frequency
    x_ticks = [[0]*len(frequency_midpoints),[0]*len(frequency_midpoints)]
    for n,x in enumerate(frequency_midpoints):
      x_ticks[0][n] = (n,str(x))
      x_ticks[1][n] = x_ticks[0][n]
    item.getAxis("bottom").setTicks(x_ticks)
    item.setXRange(-0.5, 5.5, padding=0)
    # Y axis ticks: set minor ticks to same as major
    y_ticks = [[0]*len(dB_labels),[0]*len(dB_labels)]
    for n,y in enumerate(dB_labels):
      y_ticks[0][n] = (y,str(y))
      y_ticks[1][n] = y_ticks[0][n]
    item.getAxis("left").setTicks(y_ticks)
    # fix Y axis limits, with margin:
    item.setYRange(dB_labels[0], dB_labels[-1], padding=0.05)


