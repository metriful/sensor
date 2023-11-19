"""HTTP request handler classes.

This file contains HTTP request handler classes which are used in the
web_server.py and graph_web_server.py examples.
They also use files text_web_page.html and graph_web_page.html
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

from http.server import BaseHTTPRequestHandler
from collections import deque
import struct
import pkgutil
import jinja2
from pathlib import Path
from subprocess import check_output
from . import sensor_functions as sensor
from . import sensor_constants as const


class SimpleWebpageHandler(BaseHTTPRequestHandler):
    """Make a simple text webpage to display environment data.

    The webpage is HTML and CSS only and does not use javascript.
    Periodic refresh is achieved using the 'Refresh' HTTP header.
    """

    the_web_page = "Awaiting data..."
    air_data = None
    air_quality_data = None
    sound_data = None
    light_data = None
    particle_data = None
    interpreted_AQI_accuracy = None
    interpreted_AQI_value = None
    refresh_period_seconds = 3
    template = jinja2.Environment(
            loader=jinja2.FileSystemLoader(Path(__file__).parent),
            autoescape=True).get_template("text_web_page.html")

    @classmethod
    def _get_http_headers(cls):
        """Make headers for HTTP response with variable refresh time."""
        return ("HTTP/1.1 200 OK\r\n"
                "Content-type: text/html\r\n"
                "Connection: close\r\n"
                f"Refresh: {cls.refresh_period_seconds}\r\n\r\n")

    def do_GET(self):
        """Implement the HTTP GET method."""
        self.wfile.write(
            bytes(self._get_http_headers() + self.the_web_page, "utf8"))

    def do_HEAD(self):
        """Implement the HTTP HEAD method."""
        self.wfile.write(bytes(self._get_http_headers(), "utf8"))

    @classmethod
    def assemble_web_page(cls, readout_time_and_date=None):
        """Create the updated webpage, for serving to all clients."""
        cls._interpret_data()
        cls.the_web_page = cls.template.render(
            air_data=cls.air_data, air_quality_data=cls.air_quality_data,
            sound_data=cls.sound_data, light_data=cls.light_data,
            particle_data=cls.particle_data,
            interpreted_AQI_accuracy=cls.interpreted_AQI_accuracy,
            interpreted_AQI_value=cls.interpreted_AQI_value,
            sound_band_mids_Hz=const.sound_band_mids_Hz,
            readout_time_and_date=readout_time_and_date)

    @classmethod
    def _interpret_data(cls):
        if cls.air_quality_data is not None:
            cls.interpreted_AQI_accuracy = sensor.interpret_AQI_accuracy(
                    cls.air_quality_data['AQI_accuracy'])
            cls.interpreted_AQI_value = sensor.interpret_AQI_value(
                cls.air_quality_data['AQI'])


class GraphWebpageHandler(BaseHTTPRequestHandler):
    """Make a web page with graphs to display environment data."""

    the_web_page = pkgutil.get_data(__name__, 'graph_web_page.html')
    data_period_seconds = 3
    error_response_HTTP = "HTTP/1.1 400 Bad Request\r\n\r\n"
    data_header = ("HTTP/1.1 200 OK\r\n"
                   "Content-type: application/octet-stream\r\n"
                   "Connection: close\r\n\r\n")
    page_header = ("HTTP/1.1 200 OK\r\n"
                   "Content-type: text/html\r\n"
                   "Connection: close\r\n\r\n")

    def do_GET(self):
        """Implement the HTTP GET method."""
        if self.path == '/':
            # The web page is requested
            self.wfile.write(bytes(self.page_header, "utf8"))
            self.wfile.write(self.the_web_page)
        elif self.path == '/1':
            # A URI path of '1' indicates a request of all buffered data
            self.send_all_data()
        elif self.path == '/2':
            # A URI path of '2' indicates a request of the latest data only
            self.send_latest_data()
        else:
            # Path not recognized: send a standard error response
            self.wfile.write(bytes(self.error_response_HTTP, "utf8"))

    def do_HEAD(self):
        """Implement the HTTP HEAD method."""
        if self.path == '/':
            self.wfile.write(bytes(self.page_header, "utf8"))
        elif self.path == '/1' or self.path == '/2':
            self.wfile.write(bytes(self.data_header, "utf8"))
        else:
            self.wfile.write(bytes(self.error_response_HTTP, "utf8"))

    def send_all_data(self):
        """Respond to client request by sending all buffered data."""
        self.wfile.write(bytes(self.data_header, "utf8"))
        # First send the time period, so the web page knows
        # when to do the next request
        self.wfile.write(struct.pack('H', self.data_period_seconds))
        # Send particle sensor type
        self.wfile.write(struct.pack('B', sensor.PARTICLE_SENSOR))
        # Send choice of temperature unit
        self.wfile.write(struct.pack('B', int(sensor.USE_FAHRENHEIT)))
        # Send the length of the data buffers (the number of values
        # of each variable)
        self.wfile.write(struct.pack('H', len(self.temperature)))
        # Send the data in the specific order:
        for p in [self.AQI, self.temperature, self.pressure, self.humidity,
                  self.SPL, self.illuminance, self.bVOC, self.particle]:
            self.wfile.write(struct.pack(str(len(p)) + 'f', *p))

    def send_latest_data(self):
        """Respond to client request by sending only the most recent data."""
        self.wfile.write(bytes(self.data_header, "utf8"))
        # Send the most recent values, if buffers are not empty
        if self.temperature:
            data = [self.AQI[-1], self.temperature[-1], self.pressure[-1],
                    self.humidity[-1], self.SPL[-1], self.illuminance[-1],
                    self.bVOC[-1]]
            if self.particle:
                data.append(self.particle[-1])
            self.wfile.write(struct.pack(str(len(data)) + 'f', *data))

    @classmethod
    def set_buffer_length(cls, buffer_length):
        """Create a FIFO data buffer for each variable."""
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


def get_IP_addresses():
    """Get this computer's IP addresses."""
    ips = [x.strip() for x in check_output(
        ['hostname', '-I']).decode().strip().split('\n')]
    if len(ips) == 1 and ips[0] == '':
        return []
    else:
        return ips
