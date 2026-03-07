#include "led_display.h"
#include "snake_game.h"
#include <FastLED.h>

#define LED_PIN   32
#define NUM_LEDS  (GRID_SIZE * GRID_SIZE)  // 64
#define BRIGHT    150   // 0-255, safe for USB power

// Colors
#define COL_HEAD  CRGB(0,   230, 255)   // bright cyan
#define COL_BODY  CRGB(0,   80,  30)    // dim green
#define COL_EMPTY CRGB::Black

static CRGB leds[NUM_LEDS];

// Column-major serpentine: the LED strip runs DOWN column 0, UP column 1, etc.
// This is the standard wiring for most 8x8 WS2812B matrix panels.
//
// If movement still looks wrong, try one of these alternatives:
//   All columns top→bottom (no serpentine):  return x * GRID_SIZE + y;
//   Row-major serpentine (rows, not columns): return (y%2==0) ? y*GRID_SIZE+x : y*GRID_SIZE+(GRID_SIZE-1-x);
//   Row-major all-forward:                   return y * GRID_SIZE + x;
static int pixelIndex(int x, int y) {
    if (x % 2 == 0)
        return x * GRID_SIZE + y;               // even columns: top → bottom
    else
        return x * GRID_SIZE + (GRID_SIZE - 1 - y); // odd columns:  bottom → top
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

    uint8_t grid[GRID_SIZE][GRID_SIZE];
    getGridState(grid);

    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
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
