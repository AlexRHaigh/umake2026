#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>

#define GRID_W 8
#define GRID_H 16
#define MAX_SNAKE_LENGTH 128
#define GAME_SPEED_MS 500

enum CellState {
    EMPTY = 0,
    SNAKE_HEAD = 1,
    SNAKE_BODY = 2,
    FOOD = 3
};

enum Direction {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
};

struct Point {
    int8_t x;
    int8_t y;
};

struct GameState {
    uint8_t grid[GRID_H][GRID_W];
    Point snake[MAX_SNAKE_LENGTH];
    uint8_t snakeLength;
    Direction direction;
    Direction lastMoved;   // direction the snake actually moved last tick
    Point food;
    uint16_t score;
    bool gameOver;
    unsigned long lastMoveTime;
};

extern GameState game;

void initGame();
void gameLoop();
void resetGame();
void setDirection(Direction dir);
bool isGameOver();
uint16_t getScore();
bool consumeRestartRequest();
void getGridState(uint8_t grid[GRID_H][GRID_W]);
void setLCD();

#endif
