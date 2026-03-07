#include <Arduino.h>
#include "snake_game.h"
#include "led_display.h"

// ── Joystick ──────────────────────────────────────────────────────────────────
#define JOY_X_PIN       39
#define JOY_Y_PIN       34
#define JOY_DEADZONE    500
#define RIGHT_SCALE     (2867.0f / 1228.0f)

int joyCenterX = 2048;
int joyCenterY = 2048;

void calibrateJoystick() {
    long sumX = 0, sumY = 0;
    const int samples = 40;
    for (int i = 0; i < samples; i++) {
        sumX += analogRead(JOY_X_PIN);
        sumY += analogRead(JOY_Y_PIN);
        delay(10);
    }
    joyCenterX = sumX / samples;
    joyCenterY = sumY / samples;
    Serial.printf("Calibrated center  X: %d  Y: %d\n", joyCenterX, joyCenterY);
}

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

    Serial.println("Leave joystick centered — calibrating...");
    calibrateJoystick();

    randomSeed(analogRead(0));
    initGame();
    setupLedDisplay();
    Serial.println("Game started!");
}

void loop() {
    if (isGameOver()) {
        showCountdown();
        resetGame();
        return;
    }
    readJoystick();
    gameLoop();
    updateLedDisplay();
}

// ── Cursor test (kept for reference) ─────────────────────────────────────────
// #include <FastLED.h>
// #define LED_PIN 32
// #define GRID_W 8
// #define GRID_H 8
// #define NUM_LEDS (GRID_W*GRID_H)
// #define BRIGHT 55
// CRGB leds[NUM_LEDS];
// int pixelIndex(int x, int y) { return (GRID_H-1-y)*GRID_W + (GRID_W-1-x); }
// int curX = 4, curY = 4;
// void setup() {
//     Serial.begin(115200); delay(1000);
//     analogReadResolution(12); analogSetAttenuation(ADC_11db);
//     calibrateJoystick();
//     FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
//     FastLED.setBrightness(BRIGHT);
//     fill_solid(leds, NUM_LEDS, CRGB::Black);
//     leds[pixelIndex(curX, curY)] = CRGB::Cyan;
//     FastLED.show();
// }
// void loop() {
//     static unsigned long lastMove = 0;
//     if (millis() - lastMove < 250) return;
//     lastMove = millis();
//     int rawDx = analogRead(JOY_X_PIN) - joyCenterX;
//     int dy = analogRead(JOY_Y_PIN) - joyCenterY;
//     int dx = (rawDx > 0) ? (int)(rawDx * RIGHT_SCALE) : rawDx;
//     if (abs(dx) <= JOY_DEADZONE && abs(dy) <= JOY_DEADZONE) return;
//     int nx = curX, ny = curY;
//     if (abs(dx) >= abs(dy)) { if (dx < 0 && curX > 0) nx--; else if (dx > 0 && curX < GRID_W-1) nx++; }
//     else                    { if (dy < 0 && curY > 0) ny--; else if (dy > 0 && curY < GRID_H-1) ny++; }
//     leds[pixelIndex(curX, curY)] = CRGB::Black;
//     curX = nx; curY = ny;
//     leds[pixelIndex(curX, curY)] = CRGB::Cyan;
//     FastLED.show();
//     Serial.printf("Cursor: (%d, %d)\n", curX, curY);
// }
