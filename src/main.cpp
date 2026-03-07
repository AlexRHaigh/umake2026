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

    if      (strcmp(msg.button, "UP")    == 0) setDirection(UP);
    else if (strcmp(msg.button, "DOWN")  == 0) setDirection(DOWN);
    else if (strcmp(msg.button, "LEFT")  == 0) setDirection(LEFT);
    else if (strcmp(msg.button, "RIGHT") == 0) setDirection(RIGHT);
}

void setupEspNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return;
    }

    // Add controller as peer on channel 1 (must match softAP channel)
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

// ── Joystick ──────────────────────────────────────────────────────────────────
#define JOY_X_PIN       34
#define JOY_Y_PIN       35
#define JOY_DEADZONE    500
#define RIGHT_SCALE     (2867.0f / 1228.0f)

int joyCenterX = 2048;
int joyCenterY = 2048;

void readJoystick() {
    int rawDx = analogRead(JOY_X_PIN) - joyCenterX;
    int dy    = analogRead(JOY_Y_PIN) - joyCenterY;
    int dx    = (rawDx > 0) ? (int)(rawDx * RIGHT_SCALE) : rawDx;

    if (abs(dx) <= JOY_DEADZONE && abs(dy) <= JOY_DEADZONE) return;

    if (abs(dx) >= abs(dy)) {
        if (dx < 0) { Serial.println("LEFT");  setDirection(LEFT);  }
        else        { Serial.println("RIGHT"); setDirection(RIGHT); }
    } else {
        if (dy < 0) { Serial.println("UP");   setDirection(UP);   }
        else        { Serial.println("DOWN"); setDirection(DOWN); }
    }
}
// ─────────────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(1000);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

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
        showCountdown();
        resetGame();
        return;
    }
    gameLoop();
    setLCD();
    updateLedDisplay();
    handleWebServer();
}