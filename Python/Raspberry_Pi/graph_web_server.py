"""Example of serving a web page with graphs, from a Raspberry Pi.

Example code for serving a web page over a local network to display
graphs showing environment data read from the Metriful MS430. A CSV
data file is also downloadable from the page.
This example is designed to run with Python 3 on a Raspberry Pi.

The web page can be viewed from other devices connected to the same
network(s) as the host Raspberry Pi, including wired and wireless
networks.

The browser which views the web page uses the Plotly javascript
library to generate the graphs. This is automatically downloaded
over the internet, or can be cached for offline use. If it is not
available, graphs will not appear but text data and CSV downloads
should still work.

NOTE: if you run, exit, then re-run this program, you may get an
"Address already in use" error. This ends after a short period: wait
one minute then retry.
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

import time
import socketserver
import sensor_package.servers as server
import sensor_package.sensor_functions as sensor
import sensor_package.sensor_constants as const

#########################################################
# USER-EDITABLE SETTINGS

# Choose how often to read and update data (every 3, 100, or 300 seconds)
# The web page can be refreshed more often but the data will not change
cycle_period = const.CYCLE_PERIOD_3_S

# The BUFFER_LENGTH parameter is the number of data points of each
# variable to store on the host. It is limited by the available host RAM.
buffer_length = 200
# Examples:
# For 16 hour graphs, choose 100 second cycle period and 576 buffer length
# For 24 hour graphs, choose 300 second cycle period and 288 buffer length

# The web page address will be:
# http://<your Raspberry Pi IP address>:8080   e.g. http://172.24.1.1:8080

# To find your Raspberry Pi's IP address:
# 1. Enter the command ifconfig in a terminal
# 2. Each available network connection displays a block of output
# 3. Ignore the "lo" output block
# 4. The IP address on each network is displayed after "inet"
#
# Example - part of an output block showing the address 172.24.1.1
#
# wlan0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
#        inet 172.24.1.1  netmask 255.255.255.0  broadcast 172.24.1.255

# END OF USER-EDITABLE SETTINGS
#########################################################

# Set up the GPIO and I2C communications bus
(GPIO, I2C_bus) = sensor.SensorHardwareSetup()

# Apply the chosen settings to the MS430
I2C_bus.write_i2c_block_data(
    sensor.i2c_7bit_address,
    const.PARTICLE_SENSOR_SELECT_REG, [sensor.PARTICLE_SENSOR])
I2C_bus.write_i2c_block_data(
    sensor.i2c_7bit_address, const.CYCLE_TIME_PERIOD_REG, [cycle_period])

# Get time period value to send to web page
if (cycle_period == const.CYCLE_PERIOD_3_S):
    server.GraphWebpageHandler.data_period_seconds = 3
elif (cycle_period == const.CYCLE_PERIOD_100_S):
    server.GraphWebpageHandler.data_period_seconds = 100
else:  # CYCLE_PERIOD_300_S
    server.GraphWebpageHandler.data_period_seconds = 300

# Set the number of each variable to be retained
server.GraphWebpageHandler.set_buffer_length(buffer_length)

# Choose the TCP port number for the web page.
port = 8080
# The port can be any unused number from 1-65535 but values below 1024
# require this program to be run as super-user as follows:
#    sudo python3 web_server.py
# Port 80 is the default for HTTP, and with this value the port number
# can be omitted from the web address. e.g. http://172.24.1.1

print("Starting the web server...")
ips = server.get_IP_addresses()
if not ips:
    print("Warning: no networks detected.")
else:
    print("Your web page will be available at:")
    for ip in ips:
        print(f"   http://{ip}:{port}")
    print("For more information on network IP addresses, "
          "run the command ifconfig in a terminal.")
print("Press ctrl-c to exit at any time.")

the_server = socketserver.TCPServer(("", port), server.GraphWebpageHandler)
the_server.timeout = 0.1

# Enter cycle mode to start periodic data output
I2C_bus.write_byte(sensor.i2c_7bit_address, const.CYCLE_MODE_CMD)

while True:

    # Respond to the web page client requests while waiting for new data
    while not GPIO.event_detected(sensor.READY_pin):
        the_server.handle_request()
        time.sleep(0.05)

    # Now read all data from the MS430 and pass to the web page

    # Air data
    server.GraphWebpageHandler.update_air_data(sensor.get_air_data(I2C_bus))

    # Air quality data
    # The initial self-calibration of the air quality data may take several
    # minutes to complete. During this time the accuracy parameter is zero
    # and the data values are not valid.
    server.GraphWebpageHandler.update_air_quality_data(
        sensor.get_air_quality_data(I2C_bus))

    # Light data
    server.GraphWebpageHandler.update_light_data(
        sensor.get_light_data(I2C_bus))

    # Sound data
    server.GraphWebpageHandler.update_sound_data(
        sensor.get_sound_data(I2C_bus))

    # Particle data
    # This requires the connection of a particulate sensor (invalid
    # values will be obtained if this sensor is not present).
    # Also note that, due to the low pass filtering used, the
    # particle data become valid after an initial initialization
    # period of approximately one minute.
    if (sensor.PARTICLE_SENSOR != const.PARTICLE_SENSOR_OFF):
        server.GraphWebpageHandler.update_particle_data(
            sensor.get_particle_data(I2C_bus, sensor.PARTICLE_SENSOR))
