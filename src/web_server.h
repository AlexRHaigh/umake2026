#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

#define STA_SSID "SnakeGame"
#define STA_PASSWORD "robot123"

void setupWiFiSTA();
void setupWebServer();
void handleWebServer();

#endif
