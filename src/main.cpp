#include <Arduino.h>
#include "snake_game.h"
#include "web_server.h"
#include "led_display.h"

// ── Joystick ──────────────────────────────────────────────────────────────────
// ADC1 pins only — ADC2 is blocked by WiFi on ESP32.
// pin 39 = VRX (X axis), pin 36 = VRY (Y axis)
#define JOY_X_PIN   39
#define JOY_Y_PIN   36
#define JOY_SAMPLES  8
// ─────────────────────────────────────────────────────────────────────────────

static void readJoystick() {
    int xTotal = 0, yTotal = 0;
    for (int i = 0; i < JOY_SAMPLES; i++) {
        xTotal += analogRead(JOY_X_PIN);
        yTotal += analogRead(JOY_Y_PIN);
    }
    int xVal = xTotal / JOY_SAMPLES;
    int yVal = yTotal / JOY_SAMPLES;

    // UP / DOWN: X axis must be in center band
    if (xVal < 2000 && xTotal > 1700) {
        if (yVal < 100)  { Serial.println("UP");   setDirection(UP);   }
        if (yVal > 3800) { Serial.println("DOWN");  setDirection(DOWN); }
    }

    // LEFT / RIGHT: Y axis must be in center band
    if (yVal < 1900 && yVal > 1700) {
        if (xVal < 100)  { Serial.println("LEFT");  setDirection(LEFT);  }
        if (xVal > 3900) { Serial.println("RIGHT"); setDirection(RIGHT); }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Snake Game Starting...");

    randomSeed(analogRead(0));

    initGame();
    Serial.println("Game initialized");

    setupLedDisplay();
    Serial.println("LED display ready");

    setupWiFiAP();
    setupWebServer();

    Serial.println("Ready! Connect to WiFi and open 192.168.4.1");
}

void loop() {
    readJoystick();
    gameLoop();
    updateLedDisplay();
    handleWebServer();
}
