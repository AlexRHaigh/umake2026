#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

#define AP_SSID "SnakeGame"
#define AP_PASSWORD "robot123"

void setupWiFiAP();
void setupWebServer();
void handleWebServer();

#endif
