#include "led_display.h"
#include "snake_game.h"
#include <FastLED.h>

#define LED_PIN   23
#define NUM_LEDS  (GRID_W * GRID_H)  // 128
#define BRIGHT    75   // 0-255, safe for USB power

// Colors
#define COL_HEAD  CRGB(0,   230, 255)   // bright cyan
#define COL_BODY  CRGB(0,   80,  30)    // dim green
#define COL_EMPTY CRGB::Black

static CRGB leds[NUM_LEDS];

// Row-major, no serpentine. Strip starts at physical bottom-right (game 7,15),
// runs right→left along each row, rows go bottom→top.
static int pixelIndex(int x, int y) {
    return (GRID_H - 1 - y) * GRID_W + (GRID_W - 1 - x);
}

// 5-wide × 7-tall pixel font for digits 3, 2, 1 (indices 0, 1, 2).
// Each byte is one row; bit7=col0 … bit0=col7.
// Digits occupy cols 2-6, rows 0-6, leaving a 1-pixel margin on all sides.
static const uint8_t DIGIT_FONT[3][8] = {

    // "3"
    {0b00111100,
     0b01111110,
     0b00000110,
     0b00111100,
     0b00000110,
     0b01111110,
     0b00111100,
     0b00000000},

    // "2"
    {0b00111100,
     0b01111110,
     0b00000110,
     0b00111100,
     0b01100000,
     0b01100000,
     0b01111110,
     0b00000000},

    // "1"
    {0b00011000,
     0b00111000,
     0b01111000,
     0b00011000,
     0b00011000,
     0b00011000,
     0b01111110,
     0b00000000},
};
static void showDigit(int digitIdx, CRGB color) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    const int yOffset = (GRID_H - 8) / 2;  // center 8-row font vertically
    for (int y = 0; y < 8; y++) {
        uint8_t row = DIGIT_FONT[digitIdx][y];
        for (int x = 0; x < GRID_W; x++) {
            if ((row >> (7 - x)) & 1) {
                leds[pixelIndex(x, y + yOffset)] = color;
            }
        }
    }
    FastLED.show();
}

void showCountdown() {
    Serial.println("3");
    showDigit(0, CRGB(200, 0, 0));    // red
    delay(1000);

    Serial.println("2");
    showDigit(1, CRGB(200, 100, 0));  // orange
    delay(1000);

    Serial.println("1");
    showDigit(2, CRGB(0, 200, 0));    // green
    delay(1000);

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void setupLedDisplay() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHT);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void updateLedDisplay() {
    // Rate-limit to ~30 fps — no point refreshing faster than the game moves
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < 33) return;
    lastUpdate = millis();

    if (isGameOver()) {
        // Alternate the whole grid between red and black
        static unsigned long lastFlash = 0;
        static bool flashOn = false;
        if (millis() - lastFlash > 300) {
            flashOn = !flashOn;
            lastFlash = millis();
        }
        fill_solid(leds, NUM_LEDS, flashOn ? CRGB(180, 0, 0) : CRGB::Black);
        FastLED.show();
        return;
    }

    // beatsin8 produces a sine wave 0-255 at the given BPM — gives food a pulse
    uint8_t foodBrightness = beatsin8(90, 80, 255);

    uint8_t grid[GRID_H][GRID_W];
    getGridState(grid);

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            CRGB color;
            switch (grid[y][x]) {
                case SNAKE_HEAD: color = COL_HEAD;                              break;
                case SNAKE_BODY: color = COL_BODY;                              break;
                case FOOD:       color = CRGB(foodBrightness, 0, 0);           break;
                default:         color = COL_EMPTY;                             break;
            }
            leds[pixelIndex(x, y)] = color;
        }
    }

    FastLED.show();
}
