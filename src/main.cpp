#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "snake_game.h"
#include "led_display.h"
#include "web_server.h"

// ── ESP-NOW ───────────────────────────────────────────────────────────────────
typedef struct {
    char button[10];
} ButtonMessage;

void onEspNowReceive(const uint8_t *mac_addr, const uint8_t *data, int len) {
    if (len < (int)sizeof(ButtonMessage)) return;
    ButtonMessage msg;
    memcpy(&msg, data, sizeof(msg));
    Serial.println(msg.button);

    if      (strcmp(msg.button, "UP")    == 0) {Serial.println("UP"); setDirection(UP);}
    else if (strcmp(msg.button, "DOWN")  == 0) {Serial.println("DOWN"); setDirection(DOWN);}
    else if (strcmp(msg.button, "LEFT")  == 0) {Serial.println("LEFT"); setDirection(LEFT);}
    else if (strcmp(msg.button, "RIGHT") == 0) {Serial.println("RIGHT"); setDirection(RIGHT);}
}

void setupEspNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }

    esp_now_peer_info_t peer = {};
    uint8_t controllerMac[] = {0x70, 0x4B, 0xCA, 0x8F, 0x01, 0x94};
    memcpy(peer.peer_addr, controllerMac, 6);
    peer.channel = 1;
    peer.ifidx   = WIFI_IF_AP;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    esp_now_register_recv_cb(onEspNowReceive);
    Serial.println("ESP-NOW ready");
    showCheckmark();
}
// ─────────────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    randomSeed(analogRead(0));
    initGame();
    setupLedDisplay();
    setupWiFiAP();
    setupWebServer();
    setupEspNow();
    Serial.println("Game started!");
}

void loop() {
    if (isGameOver()) {
        showScrollingScore(getScore());
        showCountdown();
        resetGame();
        return;
    }
    gameLoop();
    setLCD();
    updateLedDisplay();
    handleWebServer();
}