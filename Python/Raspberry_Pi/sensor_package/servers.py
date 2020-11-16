#  servers.py

#  This file contains HTTP request handler classes which are used in the
#  web_server.py and graph_web_server.py examples.

#  Copyright 2020 Metriful Ltd. 
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit 
#  https://github.com/metriful/sensor

import http.server
from collections import deque
import struct
from .sensor_functions import *

##########################################################################################

# A class for making a simple text web page showing the environment data in 
# separate category tables, using HTML and CSS. This is used in web_server.py

class SimpleWebpageHandler(http.server.SimpleHTTPRequestHandler):
  the_web_page = ""
  air_data = None
  air_quality_data = None
  sound_data = None
  light_data = None
  particle_data = None
  refresh_period_seconds = 3
  
  def do_GET(self):
    self.wfile.write(bytes(self.the_web_page, "utf8"))

  @classmethod
  def assemble_web_page(cls):
    cls.the_web_page = ("HTTP/1.1 200 OK\r\n"
    "Content-type:text/html\r\n"
    "Connection: close\r\n"
    "Refresh: {}\r\n\r\n".format(cls.refresh_period_seconds) +
    "<!DOCTYPE HTML><html><head><meta charset='UTF-8'>"
    "<title>Metriful Sensor Demo</title>"
    "<style>"
    "h1 {font-size: 3vw;}"
    "h2 {font-size: 2vw; margin-top: 4vw;}"
    "a {padding: 1vw; font-size: 2vw;}"
    "table, th, td {font-size: 2vw;}"
    "body {padding: 0vw 2vw;}"
    "th, td {padding: 0.05vw 1vw; text-align: left;}"
    "#v1 {text-align: right; width: 10vw;}"
    "#v2 {text-align: right; width: 13vw;}"
    "#v3 {text-align: right; width: 10vw;}"
    "#v4 {text-align: right; width: 10vw;}"
    "#v5 {text-align: right; width: 11vw;}"
    "</style></head>"
    "<body><h1>Indoor Environment Data</h1>")
        
    if (cls.air_data != None):
      cls.the_web_page += "<p><h2>Air Data</h2><table>"
      cls.the_web_page += ("<tr><td>Temperature</td><td id='v1'>"
      "{:.1f}</td><td>".format(cls.air_data['T']) + cls.air_data['T_unit'] + "</td></tr>")
      cls.the_web_page += ("<tr><td>Pressure</td>"
      "<td id='v1'>{}</td><td>Pa</td></tr>".format(cls.air_data['P_Pa']) +
      "<tr><td>Humidity</td><td id='v1'>{:.1f}</td><td>%</td></tr>".format(cls.air_data['H_pc']) +
      "<tr><td>Gas Sensor Resistance</td><td id='v1'>{}</td>".format(cls.air_data['G_ohm']) +
      "<td>" + OHM_SYMBOL + "</td></tr></table></p>")
    
    if (cls.air_quality_data != None):
      cls.the_web_page += "<p><h2>Air Quality Data</h2>"
      if (cls.air_quality_data['AQI_accuracy'] == 0):
        # values are not valid yet
        cls.the_web_page += ("<a>" + interpret_AQI_accuracy(cls.air_quality_data['AQI_accuracy']) +
        "</a></p>")
      else:
        cls.the_web_page += ("<table><tr><td>Air Quality Index</td>"
        "<td id='v2'>{:.1f}</td><td></td></tr>".format(cls.air_quality_data['AQI']) +
        "<tr><td>Air Quality Summary</td><td id='v2'>" +
        interpret_AQI_value(cls.air_quality_data['AQI']) + "</td><td></td></tr>"
        "<tr><td>Estimated CO" + SUBSCRIPT_2 + "</td>"
        "<td id='v2'>{:.1f}</td><td>ppm</td></tr>".format(cls.air_quality_data['CO2e']) +
        "<tr><td>Equivalent Breath VOC</td><td id='v2'>{:.2f}".format(cls.air_quality_data['bVOC']) +
        "</td><td>ppm</td></tr></table></p>")

    if (cls.sound_data != None):
      cls.the_web_page += ("<p><h2>Sound Data</h2><table>"
      "<tr><td>A-weighted Sound Pressure Level</td><td id='v3'>"
      "{:.1f}</td><td>dBA</td></tr>".format(cls.sound_data['SPL_dBA']))
      for i in range(0,SOUND_FREQ_BANDS):
        cls.the_web_page += ("<tr><td>Frequency Band " 
        "{} ({} Hz) SPL</td>".format(i+1, sound_band_mids_Hz[i]) +
        "<td id='v3'>{:.1f}</td><td>dB</td></tr>".format(cls.sound_data['SPL_bands_dB'][i]))
      cls.the_web_page += ("<tr><td>Peak Sound Amplitude</td>"
      "<td id='v3'>{:.2f}</td><td>mPa</td></tr></table></p>".format(cls.sound_data['peak_amp_mPa']))
      
    if (cls.light_data != None):
      cls.the_web_page += ("<p><h2>Light Data</h2><table>"
      "<tr><td>Illuminance</td><td id='v4'>{:.2f}</td>".format(cls.light_data['illum_lux']) +
      "<td>lux</td></tr>"
      "<tr><td>White Light Level</td><td id='v4'>{}</td>".format(cls.light_data['white']) +
      "<td></td></tr></table></p>")

    if (cls.particle_data != None):
      cls.the_web_page += ("<p><h2>Air Particulate Data</h2><table>"
      "<tr><td>Sensor Duty Cycle</td>"
      "<td id='v5'>{:.2f}</td><td>%</td></tr>".format(cls.particle_data['duty_cycle_pc']) +
      "<tr><td>Particle Concentration</td>"
      "<td id='v5'>{:.2f}".format(cls.particle_data['concentration']) +
      "</td><td>" + cls.particle_data['conc_unit'] + "</td></tr></table></p>")

    cls.the_web_page += "</body></html>"
    
  
##########################################################################################

# A class for making a web page with graphs to display environment data, using 
# the Plotly.js libray, javascript, HTML and CSS. This is used in graph_web_server.py

class GraphWebpageHandler(http.server.SimpleHTTPRequestHandler):
  data_period_seconds = 3
  error_response_HTTP = "HTTP/1.1 400 Bad Request\r\n\r\n"
  data_header = ("HTTP/1.1 200 OK\r\n"
                 "Content-type: application/octet-stream\r\n" 
                 "Connection: close\r\n\r\n")
  page_header = ("HTTP/1.1 200 OK\r\n" 
                 "Content-type: text/html\r\n" 
                 "Connection: close\r\n\r\n")

  # Respond to an HTTP GET request (no other methods are supported)
  def do_GET(self):
    if (self.path == '/'):
      # The web page is requested
      self.wfile.write(bytes(self.page_header, "utf8"))
      with open(self.webpage_filename, 'rb') as fileObj:
        for data in fileObj:
          self.wfile.write(data)
      fileObj.close()
    elif (self.path == '/1'):
      # A URI path of '1' indicates a request of all buffered data
      self.send_all_data()
    elif (self.path == '/2'):
      # A URI path of '2' indicates a request of the latest data only
      self.send_latest_data()
    else:
      # Path not recognized: send a standard error response
      self.wfile.write(bytes(self.error_response_HTTP, "utf8"))


  def send_all_data(self):
    self.wfile.write(bytes(self.data_header, "utf8"))
    # First send the time period, so the web page knows when to do the next request
    self.wfile.write(struct.pack('H', self.data_period_seconds))
    # Send temperature unit and particle sensor type, combined into one byte
    codeByte = PARTICLE_SENSOR
    if USE_FAHRENHEIT:
      codeByte = codeByte | 0x10
    self.wfile.write(struct.pack('B', codeByte))
    # Send the length of the data buffers (the number of values of each variable)
    self.wfile.write(struct.pack('H', len(self.temperature)))
    # Send the data:
    for p in [self.AQI, self.temperature, self.pressure, self.humidity, 
              self.SPL, self.illuminance, self.bVOC, self.particle]:
      self.wfile.write(struct.pack(str(len(p)) + 'f', *p))


  def send_latest_data(self):
    self.wfile.write(bytes(self.data_header, "utf8"))
    # Send the most recent value for each variable, if buffers are not empty
    if (len(self.temperature) > 0):
      data = [self.AQI[-1], self.temperature[-1], self.pressure[-1], self.humidity[-1], 
              self.SPL[-1], self.illuminance[-1], self.bVOC[-1]]
      if (len(self.particle) > 0):
        data.append(self.particle[-1])
      self.wfile.write(struct.pack(str(len(data)) + 'f', *data))


  @classmethod
  def set_webpage_filename(self, filename):
    self.webpage_filename = filename

  @classmethod
  def set_buffer_length(cls, buffer_length):
    cls.temperature = deque(maxlen=buffer_length)
    cls.pressure = deque(maxlen=buffer_length)
    cls.humidity = deque(maxlen=buffer_length)
    cls.AQI = deque(maxlen=buffer_length)
    cls.bVOC = deque(maxlen=buffer_length)
    cls.SPL = deque(maxlen=buffer_length)
    cls.illuminance = deque(maxlen=buffer_length)
    cls.particle = deque(maxlen=buffer_length)

  @classmethod
  def update_air_data(cls, air_data):
    cls.temperature.append(air_data['T'])
    cls.pressure.append(air_data['P_Pa'])
    cls.humidity.append(air_data['H_pc'])

  @classmethod
  def update_air_quality_data(cls, air_quality_data):
    cls.AQI.append(air_quality_data['AQI'])
    cls.bVOC.append(air_quality_data['bVOC'])

  @classmethod
  def update_light_data(cls, light_data):
    cls.illuminance.append(light_data['illum_lux'])

  @classmethod
  def update_sound_data(cls, sound_data):
    cls.SPL.append(sound_data['SPL_dBA'])

  @classmethod
  def update_particle_data(cls, particle_data):
    cls.particle.append(particle_data['concentration'])

