# Change Log
All notable changes to the project software and documentation will be documented in this file.

## [3.2.1] - 2024-01-04
### Fixed
- Wiring table for ESP8266 in README.md had incorrect SCL and SDA pin identification (D and GPIO numbers).

## [3.2.0] - 2023-11-19
### Added
- Support for Raspberry Pi Pico (W)
- ESPHome support for Home Assistant

### Changed
- Swap SDA and SCL pin definitions for ESP8266 in host_pin_definitions.h to match the standard defaults.
- Improve web page formatting and introduce jinja2 templating in Python.
- Improve string formatting in Arduino code.

## [3.1.0] - 2020-11-16
### Added
- Fahrenheit temperature output.
- Improved support for ESP8266.
- Support for ESP32.
- IFTTT example.
- Home Assistant example.
- Graph viewer software for cross-platform, real-time data monitoring.
- Graph webpage server example using Plotly.js
- Text webpage server added to Python examples (to match the one for Arduino).
- Updated the User Guide (v2.1) for the new examples.

### Changed
- **All software changes are backwards-compatible so old programs should still work.**
- The Raspberry Pi Python code now uses a package folder, so the import command has changed slightly.
- Arduino Nano 33 IoT now uses hardware I2C port, so the previously-used software I2C library is no longer needed. This requires a re-wire of two pins for this host only.
- The particle sensor being used (or not) is now set **once** rather than in each example code file - see readme for details.
- The I2C address in the Arduino examples is now set **once** rather than in each example code file.
- Small changes in most code files.


## [3.0.1] - 2020-10-14
### Fixed
- Arduino IoT_cloud_logging HTTP request problem.


## [3.0.0] - 2020-09-06
### Changed
- First release after hardware delivery.
- Datasheet (v2.0), user guide (v2.0), readme and code comments were edited.

