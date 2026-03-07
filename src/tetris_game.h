#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <Arduino.h>
#include "snake_game.h"  // Direction, GRID_W, GRID_H

#define TETRIS_EMPTY 0
// Board cell values 1–7 = piece type (determines color in display)

struct TetrisPiece {
    int8_t x, y;      // origin position on grid
    uint8_t type;      // 0=I, 1=O, 2=T, 3=S, 4=Z, 5=J, 6=L
    uint8_t rotation;  // 0–3
};

struct TetrisState {
    uint8_t board[GRID_H][GRID_W];
    TetrisPiece current;
    bool gameOver;
    uint16_t score;
    uint8_t level;
    uint16_t linesCleared;
    unsigned long lastFallTime;
    unsigned long fallInterval;
};

void initTetris();
void tetrisLoop();
void tetrisInput(Direction dir);  // UP=rotate, DOWN=hard drop, LEFT/RIGHT=nudge
bool isTetrisGameOver();
uint16_t getTetrisScore();
void getTetrisGridState(uint8_t out[GRID_H][GRID_W]);
void setTetrisLCD();

#endif
