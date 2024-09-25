#include "wifiFunctions.h"
#include <WiFi.h>

String getIP() {
  if (WiFi.status() == WL_CONNECTED) {
    return String(WiFi.localIP().toString());
  } else {
    return "Not connected";
  }
}

bool connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Already connected to WiFi! IP address: %s\n", WiFi.localIP().toString().c_str());
    return true;
  }

  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  Serial.println("");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Connected to the WiFi network! IP address: %s\n", WiFi.localIP().toString().c_str());
    return true;
  } else {
    Serial.println("WiFi connection failed!");
    return false;
  }
}
