/* 
   WiFi_functions.h

   This file declares functions used by examples connecting to, 
   or creating, a WiFi network.
   
   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#include "host_pin_definitions.h"
#ifdef HAS_WIFI
#ifndef WIFI_FUNCTIONS_H
#define WIFI_FUNCTIONS_H

#include <stdint.h>

void connectToWiFi(const char * SSID, const char * password);
bool createWiFiAP(const char * SSID, const char * password, IPAddress hostIP);
const char * interpret_WiFi_status(uint8_t statusCode);

#endif
#endif
