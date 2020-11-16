# Metriful MS430: Environment Sensor

<!-- PLEASE NOTE: this document is formatted in Markdown and is best read using a suitable viewer -->

<p align="center"><img src="pictures/group.png?raw=true"/></p>

[**Buy the hardware now - REDUCED PRICES.**](https://www.metriful.com/shop)

The Metriful MS430 is a low power, high accuracy, smart sensor cluster for indoor environment monitoring. It operates via a simple I2C-compatible interface and measures eighteen variables including air quality, light and sound levels.

This repository provides instructions and software examples for running the MS430 with **Raspberry Pi, Arduino, ESP8266** and **ESP32** host systems.

Code examples include interfaces to **IFTTT, Home Assistant** and **IoT cloud platforms**, as well as **real-time graph software, web servers** and **interrupt detection**.

This readme provides a quick-start guide to running the examples on various host systems.

The [**User Guide**](User_guide.pdf) gives an overview of the code examples and explains more about what the device measures. 

The [**Datasheet**](Datasheet.pdf) is a detailed specification of the electrical and communications interfaces of the MS430. 


### Contents

#### Hardware setup
- **[Handling precautions](#handling-precautions)**<br>
- **[Arduino](#arduino)**<br>
- **[Raspberry Pi](#raspberry-pi)**<br>
- **[ESP8266](#esp8266)**<br>
- **[ESP32](#esp32)**<br>
- **[Particle sensor](#connecting-and-enabling-a-particle-sensor)**<br>
#### Code example setup
- **[IoT cloud setup](#iot-cloud-setup)**<br>
- **[Graph web server](#graph-web-server)**<br>
- **[IFTTT example](#ifttt-example)**<br>
- **[Home Assistant example](#home-assistant-example)**<br>
- **[Graph viewer software](#graph-viewer-software)**<br>
- **[Fahrenheit temperatures](#fahrenheit-temperatures)**<br>
#### Other information 
- **[Case and enclosure ideas](#case-enclosure-and-mounting-ideas)**<br>
- **[Troubleshooting](#troubleshooting)**<br>
- **[Changelog](#changelog)**<br>
- **[License](#license)**<br>
- **[Disclaimer](#disclaimer)**<br>


## Handling precautions

The MS430 can be damaged by static electricity discharges. Minimize this risk by observing the following precautions:

- Handle the board by the edges
- Avoid touching any metal part of the device or circuit it connects to
- Store in the provided antistatic bag when not connected in a circuit
- Keep away from metal objects which could cause shorted connections


## Arduino

All code examples in the Arduino folder run on the Arduino Nano 33 IoT and Arduino MKR WiFi 1010, while those not requiring a network connection also run on Arduino Uno and Nano. 

### First time Arduino setup

Note that steps 1 and 2 are already complete if you have used Arduino before on your computer.

1. Download and install the [Arduino IDE](https://www.arduino.cc/en/main/software) on your computer.
2. Start the Arduino IDE for the first time. This will create a folder named **Arduino/libraries** in your user area (e.g. in the Documents folder on Windows computers).
3. Download and unzip the [Sensor repository](https://www.github.com/metriful/sensor). From this, copy the folder **Metriful_Sensor** (located within the Arduino folder) into the Arduino libraries folder in your user area. Delete any previous version you may have.

If using **Arduino Nano 33 IoT** or **Arduino MKR WiFi 1010**, also do the following:

4. Download and install the SAMD board package: in the Arduino IDE menu, go to Tools > Board > Boards Manager. Search for and install **Arduino SAMD Boards (32-bits ARM Cortex-M0+)**
5. Install the WiFiNINA package: in the Arduino IDE menu, go to Tools > Manage Libraries. Search for and install **WiFiNINA**.


### Wiring for Arduino

|    MS430 pin    |       Uno            |        Nano          |     Nano 33 IoT     |     MKR WiFi 1010   |
|:---------------:|:--------------------:|:--------------------:|:-------------------:|:-------------------:|
|        VIN      |       5V             |         5V           |        -            |         -           |
|        VDD      |        -             |         -            |        3.3V         |        VCC          |
|        GND      |       GND            |         GND          |        GND          |        GND          |
|        VPU      |      IOREF           |         5V           |        3.3V         |        VCC          |
|        SCL      |       SCL            |         A5           |        A5           |        SCL          |
|        SDA      |       SDA            |         A4           |        A4           |        SDA          |
|        LIT      |        D4            |         D4           |        A1           |        D4           |
|        SIT      |        D7            |         D7           |        A2           |        D5           |
|        RDY      |        D2            |         D2           |        D11          |        D0           |

* Arduino Nano 33 IoT used a software I2C library in previous code versions but now uses the hardware I2C pins.
* MS430 pin VDD is not used with the 5V systems (Uno and Nano) and VIN is not used with the 3.3V systems (Nano 33 IoT and MKR WiFi 1010).
* LIT/SIT connections are optional and only required if you are using light/sound interrupts.
* VPU can be supplied from any spare host digital output pin set to a high voltage state. This can be useful for hosts without enough power output pins.

### To run an example program on Arduino

1. Wire the MS430 board to the Arduino as described in the previous section.
2. Plug the Arduino into your computer via USB.
3. Start the Arduino IDE and open the chosen example code file, e.g. **simple_read_sound.ino**
4. In the Arduino IDE menu, go to Tools > Port and select the port with the Arduino attached.
5. Go to Tools > Board and select the Arduino model (Uno / Nano / Nano 33 IoT / MKR WiFi 1010).
6. Select Sketch > Upload and wait for upload confirmation.
7. Go to Tools > Serial Monitor to view the output (ensure **9600 baud** is selected in the monitor).


## Raspberry Pi

The example programs for Raspberry Pi use Python 3 and are provided in the **Raspberry_Pi** folder, within the **Python** folder.

### First time Raspberry Pi setup

This setup assumes that you are using Raspberry Pi OS. The standard OS version comes with all required Python packages already installed (except packages for the [Graph viewer software](#graph-viewer-software)). The **Lite** (command line) OS version requires package installation, as listed below.

1. If you are using Raspberry Pi OS Lite (or get missing package errors), run the following commands to install the packages needed: 
	```
	sudo apt-get update
	sudo apt install i2c-tools python3-smbus python3-rpi.gpio
	```

2. Enable I2C on your Raspberry Pi using the raspi-config utility by opening a terminal and running:
	```
	sudo raspi-config
	```
	Select **5 Interfacing Options** and then **P5 I2C**. A prompt will appear asking "Would you like the ARM I2C interface to be enabled?": select **Yes** and then exit the utility.
3. Shut-down the Raspberry Pi and disconnect the power. Wire up the hardware as described in the following section. Double-check the wiring then restart the Pi.
4. Check that the Metriful MS430 board can be detected by running: 
	```
	sudo i2cdetect -y 1
	```
	This should report the 7-bit address number **71**.
5. Download and unzip this [Sensor repository](https://www.github.com/metriful/sensor). The Raspberry Pi examples are found within the folder named **Raspberry_Pi**, inside the **Python** folder.

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

* Raspberry Pi pin numbering is [shown here](https://www.raspberrypi.org/documentation/usage/gpio/README.md).
* MS430 pin VIN is not used.
* LIT/SIT connections are optional and only required if you are using light/sound interrupts.

### To run an example Raspberry Pi program:

1. Wire the MS430 to the Pi as described in the previous section.
2. Open a terminal and change to the code examples folder **Raspberry_Pi**. 
3. Run one of the example programs using Python 3:
	```
	python3 simple_read_sound.py
	```


## ESP8266

All code examples in the Arduino folder have been tested on NodeMCU and Wemos D1 Mini, and are programmed using the Arduino IDE. 

Other ESP8266 development boards should also work but may use a different pinout and may therefore require edits to the host_pin_definitions.h file.

Note that ESP8266 does not have a hardware I2C module, so any of the normal GPIO pins can be used for the I2C bus. 

### First time ESP8266 setup

Note that steps 1 and 2 are already complete if you have used Arduino IDE before on your computer.

1. Download and install the [Arduino IDE](https://www.arduino.cc/en/main/software) on your computer.
2. Start the Arduino IDE for the first time. This will create a folder named **Arduino/libraries** in your user area (e.g. in the Documents folder on Windows computers).
3. Download and unzip the [Sensor repository](https://www.github.com/metriful/sensor). From this, copy the folder **Metriful_Sensor** (located within the Arduino folder) into the Arduino libraries folder in your user area. Remove any previous version you may have.
4. In the Arduino IDE menu, go to File > Preferences. In the box labeled "Additional Boards Manager URLs", paste the following link:
	```
	https://arduino.esp8266.com/stable/package_esp8266com_index.json
	```
	If there is already text in the box, place a comma and paste the new text after it.
5. In the Arduino IDE menu, go to Tools > Board > Boards Manager. Search for and install the package named **esp8266 by ESP8266 Community**.

### Wiring for ESP8266

|    MS430 pin    |       ESP8266        | 
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

* The **D** pin numbers refer to the usual pin labels printed on the development board. The GPIO numbers are the actual ESP8266 I/O identities.
* MS430 pin VIN is not used.
* LIT/SIT connections are optional and only required if you are using light/sound interrupts.
* VPU can be supplied from any spare host digital output pin set to a high voltage state. This can be useful for hosts without enough power output pins.

### To run an example program on ESP8266

1. Wire the MS430 board to the ESP8266 as described in the previous section.
2. Plug the ESP8266 board into your computer via USB.
3. Start the Arduino IDE and open the chosen example code file, e.g. **simple_read_sound.ino**
4. In the Arduino IDE menu, go to Tools > Port and select the port with the ESP8266 attached.
5. Go to Tools > Board and select your development board, or "Generic ESP8266 Module", or experiment until you find one that works.
6. Select Sketch > Upload and wait for upload confirmation.
7. Go to Tools > Serial Monitor to view the output (ensure **9600 baud** is selected in the monitor).


## ESP32

All code examples in the Arduino folder have been tested on DOIT DevKit v1, and are programmed using the Arduino IDE. 

Other ESP32 development boards should also work but may use a different pinout and may therefore require edits to the host_pin_definitions.h file.

### First time ESP32 setup

Note that steps 1 and 2 are already complete if you have used Arduino IDE before on your computer.

1. Download and install the [Arduino IDE](https://www.arduino.cc/en/main/software) on your computer.
2. Start the Arduino IDE for the first time. This will create a folder named **Arduino/libraries** in your user area (e.g. in the Documents folder on Windows computers).
3. Download and unzip the [Sensor repository](https://www.github.com/metriful/sensor). From this, copy the folder **Metriful_Sensor** (located within the Arduino folder) into the Arduino libraries folder in your user area. Remove any previous version you may have.
4. In the Arduino IDE menu, go to File > Preferences. In the box labeled "Additional Boards Manager URLs", paste the following link:
	```
	https://dl.espressif.com/dl/package_esp32_index.json
	```
	If there is already text in the box, place a comma and paste the new text after it.
5. In the Arduino IDE menu, go to Tools > Board > Boards Manager. Search for and install the package named **esp32 by Espressif Systems**.

### Wiring for ESP32

|    MS430 pin    |     ESP32     |
|:---------------:|:-------------:|
|        VIN      |       -       |
|        VDD      |      3V3      |
|        GND      |      GND      | 
|        VPU      |      3V3      | 
|        SCL      |      D22      |
|        SDA      |      D21      |
|        LIT      |      D18      |
|        SIT      |      D19      |
|        RDY      |      D23      |

* MS430 pin VIN is not used.
* LIT/SIT connections are optional and only required if you are using light/sound interrupts.
* VPU can be supplied from any spare host digital output pin set to a high voltage state. This can be useful for hosts without enough power output pins.

### To run an example program on ESP32

1. Wire the MS430 board to the ESP32 as described in the previous section.
2. Plug the ESP32 board into your computer via USB.
3. Start the Arduino IDE and open the chosen example code file, e.g. **simple_read_sound.ino**
4. In the Arduino IDE menu, go to Tools > Port and select the port with the ESP32 attached.
5. Go to Tools > Board and select your development board, or experiment until you find one that works.
6. Select Sketch > Upload and wait for upload confirmation.
7. Go to Tools > Serial Monitor to view the output (ensure **9600 baud** is selected in the monitor).


## Connecting and enabling a particle sensor

The MS430 is compatible with two widely-available air particle sensors: the Shinyei PPD42 and the Nova SDS011. The particle sensor is optional and only a single sensor can be connected at any time.

Both sensor models require three wire connections: **5V, GND, PRT** and a small edit to the example code.

|       | PPD42 pin number | SDS011 pin label |
|:-----:|:----------------:|:----------------:|
|   5V  |        3         |        5V        |
|  GND  |        1         |        GND       |
|  PRT  |        4         |        25um      | 

* PRT is on the MS430 board
* 5V and GND are supplied by the host system, or from a separate power supply. If using a separate power supply, the power supply GND must be connected to host GND.
* The SDS011 pin labeled "25um" is the data output for 0.3 - 10 µm particles.

5V is available from the hosts when they are powered from a USB power supply:

|      Host device      | 5V pin name/number |
|:---------------------:|:------------------:|
|     Raspberry Pi      |       Pin 2        |
|     Arduino Uno       |   5V or IOREF (*)  |
|     Arduino Nano      |        5V          | 
|  Arduino Nano 33 IoT  |       VUSB (**)    | 
| Arduino MKR WiFi 1010 |        5V          |
|       ESP8266         |  5V or Vin or VU   |
|        ESP32          |        VIN         |

(*) To obtain a third 5V output from the **Uno**: use pin number 2 on the 6-pin ICSP header

(**) To obtain 5V output on the **Nano 33 IoT**: the solder bridge labeled "VUSB" on the underside of the Arduino must be soldered closed, then use the VUSB pin.

* **Raspberry Pi** pin 9 can be used as an extra GND connection.
* Pin labels for ESP8266 and ESP32 may be different on some boards

### Enable the particle sensor in the code examples

* **Arduino**: in Metriful_sensor.h on the line:
	```
	#define PARTICLE_SENSOR PARTICLE_SENSOR_OFF
	```
	change ```PARTICLE_SENSOR_OFF``` to be either ```PARTICLE_SENSOR_PPD42``` or ```PARTICLE_SENSOR_SDS011```

* **Raspberry Pi**: in /sensor_package/sensor_functions.py on the line:
	```
	PARTICLE_SENSOR = PARTICLE_SENSOR_OFF
	```
	change ```PARTICLE_SENSOR_OFF``` to be either ```PARTICLE_SENSOR_PPD42``` or ```PARTICLE_SENSOR_SDS011```

## IoT cloud setup

<p align="center"><img src="pictures/tago.png?raw=true"/></p>

The **IoT_cloud_logging** code example shows how to send data to an Internet of Things (IoT) cloud storage account. It can be used with Arduino Nano 33 IoT, MKR WiFi 1010, ESP8266, ESP32 and Raspberry Pi host systems.

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


## Graph web server

<p align="center"><img src="pictures/graph_web_server.png?raw=true"/></p>

The **graph_web_server** example produces a web page with a set of graphs which display data stored on the host. The page can be accessed from other devices on your home network using their internet browsers. This is a good local alternative to using a cloud IoT service.

When opened in a browser, the web page will attempt to run the [Plotly](https://plotly.com/javascript/) javascript library which is used to create the graphs. If there is no internet access, the browser may be able to use a cached copy if it previously accessed the page with internet access. Otherwise, the graphs will not load and you will only see text data.

A button on the web page allows you to download the stored data as a CSV (comma separated value) text file, which can be opened with many spreadsheet applications.


## IFTTT example

The IFTTT example shows how to send data to [IFTTT.com](https://ifttt.com), which will trigger an email alert. It is compatible with IFTTT's free account.

The host device monitors some of the environment data and sends the alert when the values go outside your chosen "safe" range. The emails you receive will look like:
```
The temperature is too high.
The measurement was 25.5 °C on October 10, 2020 at 05.54PM.
Turn on the fan.
```
You can customize all parts of this message.

### Setup

1. Go to [IFTTT.com](https://ifttt.com) and sign up for a free account.
2. Click **Create** to start a new applet.
3. Click **If This (Add)**, search for service **Webhooks** and click **Receive a web request**, then **Connect**.
4. Enter an event name, e.g. **alert_event**, then click **Create trigger**.
5. Click **Then That (Add)** and search for **email**.
6. Choose either **Email** to get a new email for every alert, or choose **Email Digest** to get ONE email per day/week which contains all alerts. Then click **Connect**.
7. Enter and validate your email address when prompted.
8. Enter a Title/Subject for the emails, e.g. Environment data alert.
9. Delete all text from the Body/Message text box and paste in the following:
	```
	{{Value1}}<br><br>
	{{Value2}} on {{OccurredAt}}<br><br>
	{{Value3}}
	```
10. Click **Create action**, then **Continue**, then **Finish** to complete the applet setup.
11. Go to your IFTTT **Home** page and click on the applet you just created.
12. Click the triangle Webhooks icon, then **Documentation** at top right.
13. Copy the key (letter number sequence) that is displayed.
14. Edit the example code file **IFTTT**, pasting in this key and the event name (from step 4).
15. Run the program.


## Home Assistant example

<p align="center"><img src="pictures/home_assistant.png?raw=true"/></p>

This code example shows how to send sensor data to an installation of [Home Assistant](https://www.home-assistant.io) on your home network. These instructions cover setup, creating an automation using your data, and removing your data. 

Note: this was tested on Home Assistant OS v0.117.2

### Setup

1. Edit the program file "Home_Assistant":
	* Choose a sensor name and insert the IP address of the computer running Home Assistant.
	* Generate a "long lived access token":
		* On the Home Assistant web interface, click your username at the bottom left corner to go to your profile page.
		* At the bottom of the profile page, under **Long-Lived Access Tokens**, click **CREATE TOKEN** and copy the very long sequence of numbers/letters. 
		* Paste the token into the program file.
2. Run the program and wait a few minutes so that the first data have been sent.
3. Check that data are being received by Home Assistant:
	* Go to the **Configuration** page of Home Assistant
	* Click **Entities** in the component list
	* There should be a series of entries with names like SENSOR_NAME.temperature, SENSOR_NAME.air_quality_index, etc. Where SENSOR_NAME is the name you chose in the program file.


### Display/view the data in Home Assistant

* Add display cards to the **Overview** page dashboard - these can be text labels, gauges, graphs etc.
	1. Click the 3 dots at the top right corner and choose **Edit Dashboard**
	2. Add a card with the **+** symbol
	3. Choose **Entities**, **Gauge**, or **History graph**
	4. Add the variables using the entities dropdown list

* You can also view data graphs on the **History** page.

* If Home Assistant is rebooted, cards will show **Entity not available** (and the sensor will disappear from the entity list) until a new value is received. The data history will also reappear when this happens.


### Add automations using the sensor data

This simple example shows how to generate a notification when the temperature goes above 22 °C. Much more advanced triggers and actions can be configured.

1. On the Configuration page, go to: Automations and click the **+** to create a new Automation
2. Click **skip** on the "Create a new automation" window
3. On the "New Automation" page, click the 3 dots at the top right corner and choose **Edit as YAML**
4. Delete everything in the text box and replace with:
	```
	trigger:
	  - platform: numeric_state
		entity_id: kitchen3.temperature
		above: '22'
	action:
	  - service: persistent_notification.create
		data:
		  title: Kitchen temperature too high
		  message: 'The temperature is {{ states.kitchen3.temperature.state_with_unit }}'
	```
	Replace kitchen3 with your SENSOR_NAME chosen name and the other fields with your own values.
5. Click the save icon to finish.
6. Optional: click **execute** to test it (the action is forced to run without the trigger condition). 
7. Optional: edit it further (e.g. name, description) via the UI on the Configuration > Automations page.

### Removing entities and data from Home Assistant

* To hide data from view on the Overview page dashboard, simply edit or delete the cards. 
* To remove an entity and its data history from the system, follow this procedure:

**Initial one-time setup**

1. Install the Home Assistant SQLite add-on (via the Supervisor page > Add-on store > install "SQLite Web").
2. On the Supervisor page: click the SQLite Web icon, go to the Configuration tab and change "read_only" to **false**, then save.

**Entity removal**

1. Go to the **SQLite Web** page in the Home Assistant menu.
2. Click **events**, then open the **Query** tab.
3. In the text box put:
	```
	delete from states where entity_id = "kitchen3.temperature";
	```
	where **kitchen3.temperature** is the entity to remove. Or remove all entities with name beginning "kitchen3." using the % wildcard:
	```
	delete from states where entity_id like "kitchen3.%";
	```
	(replace kitchen3 with your SENSOR_NAME name).
4. Click the **Execute** button.
5. In the text box put:
	```
	vacuum;
	```
6. Click the **Execute** button.


## Graph viewer software

<p align="center"><img src="pictures/graph_viewer.png?raw=true"/></p>

The **graph viewer** uses a graphical interface to show environment data updating in real-time. It uses Python and runs on all major operating systems.

Note that the graph viewer does not run on Raspberry Pi OS **Lite** because there is no desktop interface.

There are two versions, to be used with Raspberry Pi or Arduino, provided in the Python folder.

1. **graph_viewer_I2C.py**

	Runs only on Raspberry Pi and communicates directly with the MS430 board which is connected to the Pi GPIO pins.

2. **graph_viewer_serial.py**

	Runs on multiple operating systems and uses serial over USB to get data from the MS430 sensor via a microcontroller board (e.g. Arduino, ESP8266, etc).

### Package installation commands

This assumes you have already installed Python3 and Pip3.

**Windows**
```
pip3 install pyqtgraph pyqt5 pyserial
```

**Linux, including Raspberry Pi**
```
pip3 install pyqtgraph pyserial
sudo apt install python3-pyqt5
```

**Extra steps for some Linux versions e.g. Ubuntu**
* Install pip3 by enabling the "Universe" software repository, then ```sudo apt install python3-pip```
* Add the user to the **dialout** group for permission to use the serial port.

### Running graph_viewer_I2C.py (Raspberry Pi)

1. Follow the usual hardware setup for Raspberry Pi and check that the MS430 board is recognized by the Pi.
2. Run the program with: ```python3 graph_viewer_I2C.py```

### Running graph_viewer_serial.py (all operating systems)

1. Follow the usual hardware setup for your microcontroller board.
2. Program the microcontroller board with either **cycle_readout.ino** or **on_demand_readout.ino**, with parameter ```printDataAsColumns = true```
3. Connect the microcontroller USB cable to your computer and close all serial monitor software.
4. Edit the Python code file so that the particle sensor and temperature unit settings match those used on the microcontroller.
5. Put the serial port name (system dependent, e.g. COM1) in the **serial_port_name** parameter in the code file.
6. Run the program with: ```python3 graph_viewer_serial.py```


## Fahrenheit temperatures

The temperature is always measured by the MS430 as a Celsius value. The software output values (printed to screen/file, or sent over the network, etc) are by default in Celsius but will be converted to Fahrenheit if the following changes are made:

* On Raspberry Pi

	In file **sensor_functions.py** set the following:
	```
	USE_FAHRENHEIT = True
	```

* On Arduino

	In file **Metriful_sensor.h** set the following:
	```
	#define USE_FAHRENHEIT
	```


## Case, enclosure and mounting ideas

The simplest arrangement is to leave the sensor board open to the air. Use bolts, PCB spacers/standoffs, adhesive foam pads, or hot glue to fix the board to a stand (e.g. a 6x4" picture frame). Wires can be hidden around the back to make it neater.

### Enclosing the board in a box or case

You can use a box or case with small holes for air and sound entry. Light entry can be through a transparent or open window hole. The following tips may help:

* Fix the board as close as possible to the box wall but without having the sensors touching the wall.

* Minimize the total air volume inside the box, so it exchanges quickly.

* Provide some holes for air and sound entry – these do not have definite requirements.

* If you use a particle sensor, its entry and exit ports need to be unobstructed and open to the surrounding air. So mount it against the box wall, with its ports lined up against holes in the box.

* The light sensor requires a minimum hole size to have the correct exposure, with the diameter depending on sensor distance from the hole. For example, having the sensor 2 mm from the casing edge requires a hole of about 6 mm diameter. More of these values are given on page 18 of this document: 
[https://www.vishay.com/docs/84367/designingveml6030.pdf](https://www.vishay.com/docs/84367/designingveml6030.pdf)

* The host releases heat which can affect the temperature reading if they are both together in the same box.
If you get a small temperature offset, you can measure it with an accurate thermometer and subtract it in the software - this is what the sensor manufacturer recommends.

* The SDS011 particle sensor has a powered fan which will affect the MS430 sound measurements. Reduce this by situating the SDS011 further away from the MS430, or with insulating material. Alternatively, you can "gate" power to the SDS011 to turn it on and off - the **particle_sensor_toggle** example and the User Guide give more information on this. 


## Troubleshooting

Having problems with the hardware or software? Please check [TROUBLESHOOTING](TROUBLESHOOTING.md) before opening a GitHub issue.


## Changelog

Changes, fixes and additions in each software release version are listed in the [CHANGELOG](CHANGELOG.md)


## License

See the [LICENSE](LICENSE.txt) file for software license rights and limitations (MIT).


## Disclaimer

This document and repository, and the products described herein, are subject to specific disclaimers set forth in the [DISCLAIMER](DISCLAIMER.txt) file.
