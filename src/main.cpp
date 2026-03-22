#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "snake_game.h"
#include "tetris_game.h"
#include "led_display.h"
#include "web_server.h"

enum GameMode { MODE_SELECT, MODE_SNAKE, MODE_TETRIS };

static volatile GameMode pendingMode = MODE_SELECT;
static GameMode currentMode = MODE_SELECT;

// ── ESP-NOW ───────────────────────────────────────────────────────────────────
typedef struct {
    char button[10];
} ButtonMessage;

void onEspNowReceive(const uint8_t *mac_addr, const uint8_t *data, int len) {
    if (len < (int)sizeof(ButtonMessage)) return;
    ButtonMessage msg;
    memcpy(&msg, data, sizeof(msg));
    Serial.println(msg.button);

    Direction dir;
    bool validDir = true;
    if      (strcmp(msg.button, "UP")    == 0) dir = UP;
    else if (strcmp(msg.button, "DOWN")  == 0) dir = DOWN;
    else if (strcmp(msg.button, "LEFT")  == 0) dir = LEFT;
    else if (strcmp(msg.button, "RIGHT") == 0) dir = RIGHT;
    else validDir = false;

    if (!validDir) return;

    switch (currentMode) {
        case MODE_SELECT:
            if (dir == LEFT)  pendingMode = MODE_SNAKE;
            if (dir == RIGHT) pendingMode = MODE_TETRIS;
            break;
        case MODE_SNAKE:
            setDirection(dir);
            break;
        case MODE_TETRIS:
            if (isTetrisGameOver()) requestRestart();
            else tetrisInput(dir);
            break;
    }
}

void setupEspNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }
    Serial.print("WiFi channel: ");
    Serial.println(WiFi.channel());

    esp_now_peer_info_t peer = {};
    uint8_t controllerMac[] = {0x70, 0x4B, 0xCA, 0x8E, 0x9F, 0x48};
    memcpy(peer.peer_addr, controllerMac, 6);
    peer.channel = 0;
    peer.ifidx   = WIFI_IF_STA;
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
    setupLedDisplay();
    setupWiFiSTA();
    setupWebServer();
    initLCD();
    setSelectLCD();
    setupEspNow();
    Serial.println("Select a game: LEFT=Snake, RIGHT=Tetris");
}

void loop() {
    // Handle pending mode transitions (set from ESP-NOW callback)
    if (pendingMode != currentMode) {
        currentMode = pendingMode;
        switch (currentMode) {
            case MODE_SNAKE:
                initGame();
                showCountdown();
                break;
            case MODE_TETRIS:
                initTetris();
                showCountdown();
                break;
            case MODE_SELECT:
                setSelectLCD();
                break;
        }
        return;
    }

    switch (currentMode) {
        case MODE_SELECT:
            showGameSelectDisplay();
            handleWebServer();
            break;

        case MODE_SNAKE:
            if (isGameOver()) {
                showScrollingScore(getScore());
                pendingMode = MODE_SELECT;
                return;
            }
            gameLoop();
            setLCD();
            updateLedDisplay();
            handleWebServer();
            break;

        case MODE_TETRIS:
            if (isTetrisGameOver()) {
                showScrollingScore(getTetrisScore());
                pendingMode = MODE_SELECT;
                return;
            }
            tetrisLoop();
            setTetrisLCD();
            updateTetrisDisplay();
            handleWebServer();
            break;
    }
}
