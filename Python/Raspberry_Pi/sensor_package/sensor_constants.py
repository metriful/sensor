"""Constant values for use in Python programs.

This file defines constant values which are used in the control
of the Metriful MS430 board and the interpretation of its output data.
All values have been taken from the MS430 datasheet.
"""

#  Copyright 2020-2023 Metriful Ltd.
#  Licensed under the MIT License - for further details see LICENSE.txt

#  For code examples, datasheet and user guide, visit
#  https://github.com/metriful/sensor

# Settings registers
PARTICLE_SENSOR_SELECT_REG = 0x07
LIGHT_INTERRUPT_ENABLE_REG = 0x81
LIGHT_INTERRUPT_THRESHOLD_REG = 0x82
LIGHT_INTERRUPT_TYPE_REG = 0x83
LIGHT_INTERRUPT_POLARITY_REG = 0x84
SOUND_INTERRUPT_ENABLE_REG = 0x85
SOUND_INTERRUPT_THRESHOLD_REG = 0x86
SOUND_INTERRUPT_TYPE_REG = 0x87
CYCLE_TIME_PERIOD_REG = 0x89

# Executable commands
ON_DEMAND_MEASURE_CMD = 0xE1
RESET_CMD = 0xE2
CYCLE_MODE_CMD = 0xE4
STANDBY_MODE_CMD = 0xE5
LIGHT_INTERRUPT_CLR_CMD = 0xE6
SOUND_INTERRUPT_CLR_CMD = 0xE7

# Read the operational mode
OP_MODE_READ = 0x8A

# Read data for whole categories
AIR_DATA_READ = 0x10
AIR_QUALITY_DATA_READ = 0x11
LIGHT_DATA_READ = 0x12
SOUND_DATA_READ = 0x13
PARTICLE_DATA_READ = 0x14

# Read individual data quantities
T_READ = 0x21
P_READ = 0x22
H_READ = 0x23
G_READ = 0x24
AQI_READ = 0x25
CO2E_READ = 0x26
BVOC_READ = 0x27
AQI_ACCURACY_READ = 0x28
ILLUMINANCE_READ = 0x31
WHITE_LIGHT_READ = 0x32
SPL_READ = 0x41
SPL_BANDS_READ = 0x42
SOUND_PEAK_READ = 0x43
SOUND_STABLE_READ = 0x44
DUTY_CYCLE_READ = 0x51
CONCENTRATION_READ = 0x52
PARTICLE_VALID_READ = 0x53

###############################################################

# I2C address of sensor board: can select using solder bridge
I2C_ADDR_7BIT_SB_OPEN = 0x71   # if solder bridge is left open
I2C_ADDR_7BIT_SB_CLOSED = 0x70  # if solder bridge is soldered closed

# Values for enabling/disabling of sensor functions
ENABLED = 1
DISABLED = 0

# Device modes
STANDBY_MODE = 0
CYCLE_MODE = 1

LIGHT_INTERRUPT_THRESHOLD_BYTES = 3
SOUND_INTERRUPT_THRESHOLD_BYTES = 2

# Frequency bands for sound level measurement
SOUND_FREQ_BANDS = 6
sound_band_mids_Hz = [125, 250, 500, 1000, 2000, 4000]
sound_band_edges_Hz = [88, 177, 354, 707, 1414, 2828, 5657]

# Cycle mode time period
CYCLE_PERIOD_3_S = 0
CYCLE_PERIOD_100_S = 1
CYCLE_PERIOD_300_S = 2

# Sound interrupt type
SOUND_INT_TYPE_LATCH = 0
SOUND_INT_TYPE_COMP = 1

# Maximum for illuminance measurement and threshold setting
MAX_LUX_VALUE = 3774

# Light interrupt type
LIGHT_INT_TYPE_LATCH = 0
LIGHT_INT_TYPE_COMP = 1

# Light interrupt polarity
LIGHT_INT_POL_POSITIVE = 0
LIGHT_INT_POL_NEGATIVE = 1

# Decoding the temperature integer.fraction value format
TEMPERATURE_VALUE_MASK = 0x7F
TEMPERATURE_SIGN_MASK = 0x80

# Particle sensor module selection:
PARTICLE_SENSOR_OFF = 0
PARTICLE_SENSOR_PPD42 = 1
PARTICLE_SENSOR_SDS011 = 2

###############################################################

# Byte lengths for each readable data quantity and data category

T_BYTES = 2
P_BYTES = 4
H_BYTES = 2
G_BYTES = 4
AIR_DATA_BYTES = 12

AQI_BYTES = 3
CO2E_BYTES = 3
BVOC_BYTES = 3
AQI_ACCURACY_BYTES = 1
AIR_QUALITY_DATA_BYTES = 10

ILLUMINANCE_BYTES = 3
WHITE_BYTES = 2
LIGHT_DATA_BYTES = 5

SPL_BYTES = 2
SPL_BANDS_BYTES = (2*SOUND_FREQ_BANDS)
SOUND_PEAK_BYTES = 3
SOUND_STABLE_BYTES = 1
SOUND_DATA_BYTES = 18

DUTY_CYCLE_BYTES = 2
CONCENTRATION_BYTES = 3
PARTICLE_VALID_BYTES = 1
PARTICLE_DATA_BYTES = 6

#############################################################################

# Unicode symbol strings
CELSIUS_SYMBOL = "°C"
FAHRENHEIT_SYMBOL = "°F"
SDS011_CONC_SYMBOL = "µg/m³"  # micrograms per cubic meter
SUBSCRIPT_2 = "₂"
OHM_SYMBOL = "Ω"
