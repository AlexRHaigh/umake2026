#include "led_display.h"
#include "snake_game.h"
#include "tetris_game.h"
#include <FastLED.h>
#include <Arduino.h>

#define LED_PIN   23
#define NUM_LEDS  (GRID_W * GRID_H)  // 128
#define BRIGHT    50   // 0-255, safe for USB power

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

void showCheckmark() {
    // ✓ shape, 2px thick, centered in 8×16 (rows 5–10)
    // bit7=col0 … bit0=col7
    static const uint8_t CHECK[6] = {
        0b00000011,  // row 5: cols 6,7
        0b00000110,  // row 6: cols 5,6
        0b00001100,  // row 7: cols 4,5
        0b10011000,  // row 8: cols 0 + cols 3,4
        0b11110000,  // row 9: cols 0,1,2,3
        0b01100000,  // row 10: cols 1,2
    };
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (int y = 0; y < 6; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if ((CHECK[y] >> (7 - x)) & 1) {
                leds[pixelIndex(x, y + 5)] = CRGB(0, 220, 0);
            }
        }
    }
    FastLED.show();
    delay(1500);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
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

// ── Scrolling score display ───────────────────────────────────────────────────
// 5×7 pixel font. Row-based: bit7=col0 (leftmost), ..., bit3=col4 (rightmost).
static const struct { char c; uint8_t rows[7]; } SCORE_FONT[] = {
    {'S', {0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70}},
    {'C', {0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70}},
    {'O', {0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70}},
    {'R', {0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88}},
    {'E', {0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8}},
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {'0', {0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70}},
    {'1', {0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70}},
    {'2', {0x70, 0x88, 0x08, 0x10, 0x20, 0x40, 0xF8}},
    {'3', {0x70, 0x88, 0x08, 0x30, 0x08, 0x88, 0x70}},
    {'4', {0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10}},
    {'5', {0xF8, 0x80, 0xF0, 0x08, 0x08, 0x88, 0x70}},
    {'6', {0x70, 0x80, 0x80, 0xF0, 0x88, 0x88, 0x70}},
    {'7', {0xF8, 0x08, 0x10, 0x20, 0x20, 0x20, 0x20}},
    {'8', {0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70}},
    {'9', {0x70, 0x88, 0x88, 0x78, 0x08, 0x88, 0x70}},
};
static const int SCORE_FONT_SIZE = sizeof(SCORE_FONT) / sizeof(SCORE_FONT[0]);

static const uint8_t* getScoreChar(char c) {
    for (int i = 0; i < SCORE_FONT_SIZE; i++) {
        if (SCORE_FONT[i].c == c) return SCORE_FONT[i].rows;
    }
    return SCORE_FONT[5].rows;  // space for unknown
}

static int getCharWidth(char c) { return c == ' ' ? 3 : 6; }  // space is 3px, others 5px glyph + 1px gap

void showScrollingScore(uint16_t score) {
    char msg[18];
    snprintf(msg, sizeof(msg), "SCORE %u", score);
    int msgLen = strlen(msg);

    // Pre-compute per-character pixel start positions
    int charStart[19];
    int totalW = 0;
    for (int i = 0; i < msgLen; i++) {
        charStart[i] = totalW;
        totalW += getCharWidth(msg[i]);
    }
    charStart[msgLen] = totalW;  // sentinel

    const int yOffset = (GRID_H - 7) / 2;   // center 7-row font in 16 rows

    // Scroll right-to-left; loops until ESP-NOW UP is received
    while (true) {
        for (int offset = 0; offset <= GRID_W + totalW; offset++) {
            if (consumeRestartRequest()) return;

            fill_solid(leds, NUM_LEDS, CRGB::Black);

            for (int displayCol = 0; displayCol < GRID_W; displayCol++) {
                int msgPixel = displayCol - (GRID_W - offset);
                if (msgPixel < 0 || msgPixel >= totalW) continue;

                // Find which character this pixel belongs to
                int ci = 0;
                while (ci < msgLen - 1 && charStart[ci + 1] <= msgPixel) ci++;

                int charCol = msgPixel - charStart[ci];
                if (msg[ci] == ' ') continue;   // space is blank
                if (charCol >= 5) continue;      // 1px gap after glyph

                const uint8_t* charRows = getScoreChar(msg[ci]);
                for (int row = 0; row < 7; row++) {
                    if ((charRows[row] >> (7 - charCol)) & 1) {
                        leds[pixelIndex(displayCol, row + yOffset)] = CRGB(255, 140, 0);
                    }
                }
            }

            FastLED.show();
            delay(70);  // ~14 pixels/sec scroll speed
        }
    }
}
// ─────────────────────────────────────────────────────────────────────────────

void setupLedDisplay() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHT);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void showGameSelectDisplay() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < 200) return;  // ~5 FPS
    lastUpdate = millis();

    uint8_t pulse = beatsin8(30, 80, 255);
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (x < 4) {
                leds[pixelIndex(x, y)] = CRGB(0, pulse, 0);                   // green: snake
            } else {
                leds[pixelIndex(x, y)] = CRGB(0, (uint8_t)(pulse * 9 / 10), pulse);  // cyan: tetris
            }
        }
    }
    FastLED.show();
}

void updateTetrisDisplay() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < 33) return;
    lastUpdate = millis();

    if (isTetrisGameOver()) {
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

    static const CRGB PIECE_COLORS[8] = {
        CRGB::Black,         // 0: empty
        CRGB(0, 230, 255),   // 1: I - cyan
        CRGB(230, 200, 0),   // 2: O - yellow
        CRGB(180, 0, 230),   // 3: T - purple
        CRGB(0, 200, 50),    // 4: S - green
        CRGB(230, 0, 0),     // 5: Z - red
        CRGB(0, 80, 230),    // 6: J - blue
        CRGB(230, 120, 0),   // 7: L - orange
    };

    uint8_t grid[GRID_H][GRID_W];
    getTetrisGridState(grid);

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            uint8_t cell = grid[y][x];
            leds[pixelIndex(x, y)] = (cell < 8) ? PIECE_COLORS[cell] : CRGB::Black;
        }
    }
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

    bool reversed = game.reverseControls;
    bool spedup = game.speedUp;
    CRGB snakeHead = reversed ? CRGB(220, 0, 220) : spedup ? CRGB(255, 220, 0) : COL_HEAD;
    CRGB snakeBody = reversed ? CRGB(80,  0,  80) : spedup ? CRGB(100, 80,  0) : COL_BODY;

    uint8_t grid[GRID_H][GRID_W];
    getGridState(grid);

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            CRGB color;
            switch (grid[y][x]) {
                case SNAKE_HEAD: color = snakeHead;                             break;
                case SNAKE_BODY: color = snakeBody;                             break;
                case FOOD:       color = CRGB(foodBrightness, 0, 0);           break;
                case WALL:       color = CRGB(80, 0, 180);                     break;
                default:         color = COL_EMPTY;                             break;
            }
            leds[pixelIndex(x, y)] = color;
        }
    }

    FastLED.show();
}
