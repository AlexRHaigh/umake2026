#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H

#include <Arduino.h>

void setupLedDisplay();
void updateLedDisplay();
void showCountdown();
void showCheckmark();
void showScrollingScore(uint16_t score);
void showGameSelectDisplay();
void updateTetrisDisplay();

#endif
