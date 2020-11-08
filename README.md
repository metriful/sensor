# Metriful MS430: Environment Sensor

<!-- PLEASE NOTE: this document is formatted in Markdown and should be read using a suitable viewer -->

![](sensor_pcb.png)

The Metriful MS430 is a low power, high accuracy, smart sensor cluster for indoor environment monitoring. It is operated via a simple I2C-compatible interface and measures eighteen variables including air quality, light and sound levels.

This repository provides instructions and software examples for running the MS430 module with Raspberry Pi, Arduino and NodeMCU host systems. These are also useful starting points for development with other hosts.

The code examples demonstrate various ways of using the module. This includes basic control/readout, saving data to files and IoT cloud storage. Detailed comments explain each part of the programs.

The [**User Guide**](User_guide.pdf) covers hardware setup in more detail, gives an overview of the code examples and explains more about what the device measures. 

The [**Datasheet**](Datasheet.pdf) is a detailed specification of the electrical and communications interfaces of the MS430. 

You can also [visit the product homepage.](https://www.sensor.metriful.com)

### Contents
**[Handling precautions](#handling-precautions)**<br>
**[Use with Arduino](#use-with-arduino)**<br>
**[Use with Raspberry Pi](#use-with-raspberry-pi)**<br>
**[Use with NodeMCU](#use-with-nodemcu)**<br>
**[IoT cloud setup](#iot-cloud-setup)**<br>
**[License](#license)**<br>
**[Disclaimer](#disclaimer)**<br>


## Handling precautions

The MS430 can be damaged by static electricity discharges. Minimize this risk by observing the following precautions:

- Handle the board by the edges
- Avoid touching any metal part of the device or circuit it connects to
- Store in the provided antistatic bag when not connected in a circuit
- Keep away from metal objects which could cause shorted connections


## Use with Arduino

All code examples in the Arduino folder run on the Arduino Nano 33 IoT and Arduino MKR WiFi 1010, while those not requiring a network connection also run on Arduino Uno and Nano. 

### First time Arduino setup

Note that steps 1 and 2 are already complete if you have used Arduino before on your computer.

1. Download and install the [Arduino IDE](https://www.arduino.cc/en/main/software) on your computer.
2. Start the Arduino IDE for the first time. This will create a folder named **Arduino/libraries** in your user area (e.g. in the Documents folder on Windows computers).
3. Download and unzip the [Sensor repository](https://www.github.com/metriful/sensor). From this, copy the folder **Metriful_Sensor** (located within the Arduino folder) into the Arduino libraries folder in your user area.

If using **Arduino Nano 33 IoT** or **Arduino MKR WiFi 1010**, also do the following:

4. Download and install the SAMD board package: in the Arduino IDE menu, go to Tools > Board > Boards Manager (top of list). Search for and install **Arduino SAMD Boards (32-bits ARM Cortex-M0+)**
5. Install the WiFiNINA package: in the Arduino IDE menu, go to Tools > Manage Libraries. Search for and install **WiFiNINA**.

If using **Arduino Nano 33 IoT**, also do the following:

6. Download the [SlowSoftWire library](https://github.com/felias-fogg/SlowSoftWire). Unzip it and move it into the Arduino libraries folder in your user area.
7. Download the [SlowSoftI2CMaster library](https://github.com/felias-fogg/SlowSoftI2CMaster). Unzip it and move it into the Arduino libraries folder in your user area.

### Wiring for Arduino

|    MS430 pin    |       Uno            |        Nano          |     Nano 33 IoT     |     MKR WiFi 1010   |
|:---------------:|:--------------------:|:--------------------:|:-------------------:|:-------------------:|
|        VIN      |       5V             |         5V           |        -            |         -           |
|        VDD      |        -             |         -            |        3.3V         |        VCC          |
|        GND      |       GND            |         GND          |        GND          |        GND          |
|        VPU      |      IOREF           |         5V           |        3.3V         |        VCC          |
|        SCL      |       SCL            |         A5           |        A3           |        SCL          |
|        SDA      |       SDA            |         A4           |        A0           |        SDA          |
|        LIT      |        D4            |         D4           |        A1           |        D4           |
|        SIT      |        D7            |         D7           |        A2           |        D5           |
|        RDY      |        D2            |         D2           |        D11          |        D0           |

* MS430 pin VDD is not used with 5 V systems and VIN is not used with 3.3 V systems.

* If using the PPD42 particle sensor, note that its pin numbering runs from right to left, and connect:
	* Arduino 5V pin to PPD42 pin 3
	* Arduino GND pin to PPD42 pin 1
	* PPD42 pin 4 to MS430 pin PRT

* If using the SDS011 particle sensor, connect:
	* Arduino 5V pin to SDS011 pin "5V"
	* Arduino GND pin to SDS011 pin "GND"
	* SDS011 pin "25um" to MS430 pin PRT

* To obtain 5V output on the Nano 33 IoT: the solder bridge labeled "VUSB" on the underside of the Arduino must be soldered closed, then use the VUSB pin.
* To obtain a third 5V output on the Uno: use pin number 2 on the 6-pin ICSP header
* With all hosts, VPU can be supplied from any host digital output pin set to a high voltage state. This can be useful for hosts without enough power output pins.

### To run an example program on Arduino

1. Wire the MS430 board to the Arduino as described in the previous section.
2. Plug the Arduino into your computer via USB.
3. Start the Arduino IDE and open the chosen example code file, e.g. **simple_read_sound.ino**
4. In the Arduino IDE menu, go to Tools > Port and select the port with the Arduino attached.
5. Go to Tools > Board and select the Arduino model (Uno / Nano / Nano 33 IoT / MKR WiFi 1010).
6. Select Sketch > Upload and wait for upload confirmation.
7. Go to Tools > Serial Monitor to view the output (ensure **9600 baud** is selected in the monitor).


## Use with Raspberry Pi

The example programs for Raspberry Pi use Python 3 and are provided in the **Raspberry_Pi** folder.

### First time Raspberry Pi setup

This setup assumes that you are using the full version of Raspberry Pi OS (Raspbian) Buster, which comes with all required Python packages already installed (the packages used are: **RPi.GPIO**, **smbus** and **requests**). If you are using the minimal Raspberry Pi OS Lite image instead you will need to install the following packages:

```
sudo apt-get update
sudo apt-get install -y i2c-tools git python3-dev python3-pip python3-smbus RPi.GPIO
```

1. Enable I2C on your Raspberry Pi using the raspi-config utility by opening a terminal and running:
	```
	sudo raspi-config
	```
	Select **5 Interfacing Options** and then **P5 I2C**. A prompt will appear asking "Would you like the ARM I2C interface to be enabled?": select **Yes** and then exit the utility.
2. Shut-down the Raspberry Pi and disconnect the power. Wire up the hardware as described in the following section. Double-check the wiring then restart the Pi.
3. Check that the Metriful MS430 board can be detected by running: 
	```
	sudo i2cdetect -y 1
	```
	This should report the 7-bit address number **71**.
4. Download and unzip this [Sensor repository](https://www.github.com/metriful/sensor). The Raspberry Pi examples are found within the folder named **Raspberry_Pi**.

### Wiring for Raspberry Pi

|      MS430 pin    | Raspberry Pi pin number | Raspberry Pi pin name | 
|:-----------------:|:-----------------------:|:---------------------:|
|         VIN       |           -             |           -           |
|         VDD       |          1              |       3V3 power       |
|         GND       |          6              |        Ground         | 
|         VPU       |          17             |      3V3 power        | 
|         SCL       |          5              |      GPIO 3 (SCL)     |
|         SDA       |          3              |      GPIO 2 (SDA)     |
|         LIT       |          7              |        GPIO 4         | 
|         SIT       |          8              |        GPIO 14        |
|         RDY       |          11             |        GPIO 17        |

* MS430 pin VIN is not used.

* If using the PPD42 particle sensor, note that its pin numbering runs from right to left, and connect:
	* Pi pin 2 (5V) to PPD42 pin 3
	* Pi pin 9 (Ground) to PPD42 pin 1
	* PPD42 pin 4 to MS430 pin PRT

* If using the SDS011 particle sensor, connect:
	* Pi pin 2 (5V) to SDS011 pin "5V"
	* Pi pin 9 (Ground) to SDS011 pin "GND"
	* SDS011 pin "25um" to MS430 pin PRT

### To run an example Raspberry Pi program:

1. Wire the MS430 to the Pi as described in the previous section.
2. Open a terminal and change to the code examples folder **Raspberry_Pi**. 
3. Run one of the example programs using Python 3:
	```
	python3 simple_read_sound.py
	```


## Use with NodeMCU

All code examples in the Arduino folder run on the NodeMCU (ESP8266) and are programmed using the Arduino IDE.

### First time NodeMCU setup

Note that steps 1 and 2 are already complete if you have used Arduino before on your computer.

1. Download and install the [Arduino IDE](https://www.arduino.cc/en/main/software) on your computer.
2. Start the Arduino IDE for the first time. This will create a folder named **Arduino/libraries** in your user area (e.g. in the Documents folder on Windows computers).
3. Download and unzip the [Sensor repository](https://www.github.com/metriful/sensor). From this, copy the folder **Metriful_Sensor** (located within the Arduino folder) into the Arduino libraries folder in your user area.
4. In the Arduino IDE menu, go to File > Preferences. In the box labeled "Additional Boards Manager URLs", paste the following link:
	```
	https://arduino.esp8266.com/stable/package_esp8266com_index.json
	```
5. In the Arduino IDE menu, go to Tools > Board > Boards Manager. Search for and install the package named **esp8266 by ESP8266 Community**.

### Wiring for NodeMCU

|    MS430 pin    |       NodeMCU        | 
|:---------------:|:--------------------:|
|        VIN      |        -             |
|        VDD      |       3V3            |
|        GND      |       GND            | 
|        VPU      |       3V3            | 
|        SCL      |    D2 (GPIO 4)       |
|        SDA      |    D1 (GPIO 5)       |
|        LIT      |    D3 (GPIO 0)       |
|        SIT      |    D5 (GPIO 14)      |
|        RDY      |    D6 (GPIO 12)      |

* MS430 pin VIN is not used.
	
* If using the PPD42 particle sensor, note that its pin numbering runs from right to left, and connect:
	* NodeMCU Vin (5V) pin to PPD42 pin 3
	* NodeMCU GND pin to PPD42 pin 1
	* PPD42 pin 4 to MS430 pin PRT

* If using the SDS011 particle sensor, connect:
	* NodeMCU Vin (5V) pin to SDS011 pin "5V"
	* NodeMCU GND pin to SDS011 pin "GND"
	* SDS011 pin "25um" to MS430 pin PRT

### To run an example program on NodeMCU

1. Wire the MS430 board to the NodeMCU as described in the previous section.
2. Plug the NodeMCU into your computer via USB.
3. Start the Arduino IDE and open the chosen example code file, e.g. **simple_read_sound.ino**
4. In the Arduino IDE menu, go to Tools > Port and select the port with the NodeMCU attached.
5. Go to Tools > Board and select "Generic ESP8266 Module"
6. Select Sketch > Upload and wait for upload confirmation.
7. Go to Tools > Serial Monitor to view the output (ensure **9600 baud** is selected in the monitor).


## IoT cloud setup

The **IoT_cloud_logging** code example shows how to send data to an Internet of Things (IoT) cloud storage account. It can be used with Arduino Nano 33 IoT, MKR WiFi 1010, NodeMCU and Raspberry Pi host systems.

IoT cloud hosting is available from many providers around the world. Some offer free accounts (with storage or access limits) for non-commercial purposes. The IoT cloud logging example gives a choice of two providers, [Tago.io](https://tago.io) and [Thingspeak.com](https://thingspeak.com). The following sections give a brief overview of how to set up free accounts with these providers: for further information see the relevant provider website.

### Tago cloud

The steps required to set up Tago for the IoT cloud logging code example are:

1. Register for a free account at [Tago.io](https://tago.io)
2. In the **Devices** menu, click **Add Device**.
3. Select **Custom HTTPS** (HTTP) as the device type.
4. Choose a device name (e.g. Indoor Environment Monitor) and click **Create device**
5. On the **General Information** tab of the new device, in the section **Token & Serial Number**, click the eye icon to reveal the token (a long sequence of letters, numbers and hyphens). Copy this token.
6. Paste the token into the IoT_cloud_logging example code as the variable **TAGO_DEVICE_TOKEN_STRING** and set the variable **useTagoCloud** as **true**.
7. Run the IoT cloud logging example code on the internet-connected host for a few minutes to ensure at least one set of data has been logged.
8. Verify that data are being stored in the Tago cloud by viewing the bucket’s **Variables** tab. This should show the following short names for the environment data variables:
	* **temperature**
	* **pressure**
	* **humidity**
	* **aqi** (air quality index, AQI)
	* **aqi_string** (a text interpretation of the AQI)
	* **bvoc** (equivalent breath VOC concentration)
	* **spl** (the A-weighted Sound Pressure Level)
	* **peak_amp** (the peak sound amplitude)
	* **illuminance**
	* **particulates** (air particle concentration)
9. Create a Tago dashboard for viewing the data: click the **+** to add a new dashboard, choose its name and click **save**.
10. Add widgets of various types to the dashboard (e.g. **Line** for a simple graph, **Card** to display text or numbers). 
11. Configure each widget: the minimum configuration is to choose the **Variable** which it displays. It is also possible to edit graph and axis titles, coloring, calculate formulas, etc.
12. Set graph displayed time period, e.g. for 24 hours: on the **Data Range & Format** tab of the chart settings, input **864** as the **maximum number of points to be displayed**. This assumes that the data are logged every 100 seconds (there are 86,400 seconds in 24 hours).
13. Create a public link for sharing your dashboard: click the three dots next to your dashboard name on the left-hand list. Choose “share” then copy the link displayed under the “share public” tab.

### Thingspeak cloud

The steps required to set up Thingspeak for the IoT cloud logging code example are:

1. Register for a free account at [Thingspeak.com](https://thingspeak.com)
2. On the **My Channels** page, click the **New Channel** button
3. Choose a channel name (e.g. Indoor Environment Data)
4. Enable all eight fields (check boxes)
5. Put the following labels in the field name boxes (the order is important):
	* Field 1: **Temperature / C**
	* Field 2: **Pressure / Pa**
	* Field 3: **Humidity / %**
	* Field 4: **Air Quality Index**
	* Field 5: **Breath VOC / ppm**
	* Field 6: **Sound Level / dBA SPL**
	* Field 7: **Illuminance / lx**
	* Field 8: **Particle Concentration**
6. Click **Save Channel**. The channel will show eight (initially empty) graphs.
7. To set graph time periods to 24 hours: on each graph click the pencil icon, delete **60** from the **Results** box and put **1** in the **Days** box, then click **Save**. This changes the graph period from the last 60 values to the last 1 day. This must be done separately for both the private and public view if the channel is shared publicly.
8. The channel can be made public, if desired, from the Thingspeak **Sharing** tab.
9. Go to the **API Keys** tab and copy the Write API Key (a sequence of letters and numbers).
10. Paste the API key into the Metriful IoT cloud logging example code as the variable **THINGSPEAK_API_KEY_STRING** and set the variable **useTagoCloud** as **false**.

## License

See the [LICENSE](LICENSE.txt) file for software license rights and limitations (MIT).

## Disclaimer

This document and repository, and the products described herein, are subject to specific disclaimers set forth in the [DISCLAIMER](DISCLAIMER.txt) file.
