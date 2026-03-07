#include <Arduino.h>
#include <WiFi.h>
#include "snake_game.h"
#include "led_display.h"
#include "web_server.h"

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