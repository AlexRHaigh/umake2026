#include "web_server.h"
#include "web_page.h"
#include "snake_game.h"

#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

static void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

static void handleStatus() {
    uint8_t grid[GRID_H][GRID_W];
    getGridState(grid);

    String json = "{\"grid\":[";

    for (int y = 0; y < GRID_H; y++) {
        json += "[";
        for (int x = 0; x < GRID_W; x++) {
            json += String(grid[y][x]);
            if (x < GRID_W - 1) json += ",";
        }
        json += "]";
        if (y < GRID_H - 1) json += ",";
    }

    json += "],\"score\":";
    json += String(getScore());
    json += ",\"gameOver\":";
    json += isGameOver() ? "true" : "false";
    json += "}";

    server.send(200, "application/json", json);
}

static void handleRestart() {
    resetGame();
    server.send(200, "application/json", "{\"success\":true}");
}

void setupWiFiAP() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.println();
    Serial.println("Access Point Started");
    Serial.print("SSID: ");
    Serial.println(AP_SSID);
    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void setupWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/restart", HTTP_POST, handleRestart);

    server.begin();
    Serial.println("Web server started on port 80");
}

void handleWebServer() {
    server.handleClient();
}
