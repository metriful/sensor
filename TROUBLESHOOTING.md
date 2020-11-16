# Troubleshooting

<!-- PLEASE NOTE: this document is formatted in Markdown and is best read using a suitable viewer -->

This file lists solutions for some common problems. Please check it before opening a GitHub issue.

### Contents
**[Standard checks](#standard-checks)**<br>
**[ESP8266 problems](#esp8266-problems)**<br>
**[Arduino Nano 33 IoT problems](#arduino-nano-33-iot-problems)**<br>
**[WiFi connection problems](#wifi-connection-problems)**<br>
**[Particle sensor problems](#particle-sensor-problems)**<br>
**[Slow air quality accuracy change](#slow-air-quality-accuracy-change)**<br>
**[Temperature measurement is too high](#temperature-measurement-is-too-high)**<br>


## Standard checks

Most problems can be resolved by following these steps:

1. Check that you can run a simple program on your host system **without the MS430 board** e.g. the blink demo on Arduino.
2. Ensure you have the most recent sensor code and instructions from our [GitHub repository](https://www.github.com/metriful/sensor)
3. Remove all wire connections and re-wire carefully.
4. If you have edited the code, go back to the original version and ensure it still works.


## ESP8266 problems

There are many different development boards which use the ESP8266 module. Some may have different pin labels, or have different pin positions, so you may need to research your board or (rarely) edit the host_pin_definitions.h file.

The ESP8266 does not have a hardware I2C module, so any of the normal GPIO pins can be used for the I2C bus. 


## Arduino Nano 33 IoT problems

The GitHub code releases **before v3.1.0** used a software I2C library for the Nano 33 IoT. The code now uses the hardware I2C module, with different pins - please follow the readme file to re-wire your setup.


## WiFi connection problems

If WiFi connection never succeeds, check the following:
* SSID and password are correct
* The WiFi access point is functioning and within range
* WiFi library, Arduino board package, and board firmware versions are up-to-date and compatible
* Power supply is sufficient
* Arduino WiFi antenna is not damaged (very easy to damage on the Arduino Nano 33 IoT)


## Particle sensor problems

### Measured value does not change
* Check wiring 
* Check the input voltage - the "5V" must be 4.7-5.3 V
* If using a separate 5V source, the source GND must be connected to the host/MS430 GND.

### Measured value fluctuates a lot
* This is typical for particle sensors, especially the PPD42
* Check the input voltage - the "5V" must be 4.7-5.3 V
* Using a separate, regulated 5V supply can reduce the fluctuation.


## Slow air quality accuracy change

The air quality measurement accuracy can be slow to increase, especially with new sensors, for two reasons:

1. The air quality sensor uses a small internal heater - this will gradually evaporate off impurities when used after a period of storage. 

2. The analysis algorithm self-calibrates in response to a range of air qualities, which must be provided to the sensor after it starts monitoring.

To speed up this process:

* Run any code example which repeatedly measures the air quality, with 3 second cycle selected (e.g. cycle_readout, web_server, log_data_to_file)
* Keep it running as long as possible, ideally at least 48 hours
* If the accuracy is low (0 or 1) after running for an hour, expose the sensor to polluted air - a solvent vapor such as from a marker pen is ideal.

In normal use the accuracy does not remain on 3 (highest) all the time but instead will periodically decrease/increase as calibration is ongoing.


## Temperature measurement is too high

* The temperature sensor measurement may have a small offset compared to the true temperature.

* The offset is different for each sensor but should remain mostly constant, so you can subtract it in your software after comparing with an accurate thermometer.

* A larger offset will result if you put the sensor board in a case, especially if together with the host system. Hosts (and particle sensors) create heat, so should be separated or insulated from the sensor board. 



## Support

If the information here does not help, please open a GitHub issue.

