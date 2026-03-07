#include "web_server.h"
#include "snake_game.h"
#include <esp_wifi.h>

#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

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

    char effectsBuf[64];
    getActiveEffects(effectsBuf, sizeof(effectsBuf));
    json += ",\"effects\":\"";
    json += effectsBuf;
    json += "\"";

    json += "}";

    server.send(200, "application/json", json);
}

static void handleRestart() {
    resetGame();
    server.send(200, "application/json", "{\"success\":true}");
}

static void handleEffect() {
    String body = server.arg("plain");
    applyEffect(body.c_str());
    server.send(200, "application/json", "{\"success\":true}");
}

void setupWiFiSTA() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(STA_SSID, STA_PASSWORD);

    Serial.print("Connecting to ");
    Serial.print(STA_SSID);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi connected");
    esp_wifi_set_ps(WIFI_PS_NONE);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setupWebServer() {
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/restart", HTTP_POST, handleRestart);
    server.on("/effect", HTTP_POST, handleEffect);

    server.begin();
    Serial.println("Web server started on port 80");
}

void handleWebServer() {
    server.handleClient();
}
