#include <Arduino.h>
#include "snake_game.h"
#include "web_server.h"

// ── Joystick pins ─────────────────────────────────────────────────────────────
// Swap VRX/VRY if your physical up/down and left/right are transposed.
#define JOY_VRX_PIN   34
#define JOY_VRY_PIN   32

// ADC thresholds (12-bit, 0-4095).  Center rests ~1900.
#define JOY_DEAD_LOW   1400   // below this = "active low"  on that axis
#define JOY_DEAD_HIGH  2600   // above this = "active high" on that axis
#define JOY_ACTIVE_LOW  400   // firmly pushed to minimum
#define JOY_ACTIVE_HIGH 3700  // firmly pushed to maximum

// Number of ADC samples averaged per read to reduce noise
#define JOY_SAMPLES 8
// ─────────────────────────────────────────────────────────────────────────────

static void readJoystick() {
    int xSum = 0, ySum = 0;
    for (int i = 0; i < JOY_SAMPLES; i++) {
        xSum += analogRead(JOY_VRX_PIN);
        ySum += analogRead(JOY_VRY_PIN);
    }
    int x = xSum / JOY_SAMPLES;
    int y = ySum / JOY_SAMPLES;

    bool xCenter = (x > JOY_DEAD_LOW && x < JOY_DEAD_HIGH);
    bool yCenter = (y > JOY_DEAD_LOW && y < JOY_DEAD_HIGH);

    // Prioritise the axis that is further from center
    int xDev = abs(x - 2048);
    int yDev = abs(y - 2048);

    if (xDev > yDev) {
        // Horizontal dominant
        if (yCenter) {
            if (x < JOY_ACTIVE_LOW)  setDirection(LEFT);
            if (x > JOY_ACTIVE_HIGH) setDirection(RIGHT);
        }
    } else {
        // Vertical dominant
        if (xCenter) {
            if (y < JOY_ACTIVE_LOW)  setDirection(UP);
            if (y > JOY_ACTIVE_HIGH) setDirection(DOWN);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Snake Game Starting...");

    randomSeed(analogRead(0));

    initGame();
    Serial.println("Game initialized");

    setupWiFiAP();
    setupWebServer();

    Serial.println("Ready! Connect to WiFi and open 192.168.4.1");
}

void loop() {
    readJoystick();
    gameLoop();
    handleWebServer();
}
