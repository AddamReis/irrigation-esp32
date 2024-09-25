#ifndef WIFIFUNCTIONS_H
#define WIFIFUNCTIONS_H

#include <Arduino.h>

//Wi-Fi
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

bool connectToWiFi();
String getIP();

#endif
