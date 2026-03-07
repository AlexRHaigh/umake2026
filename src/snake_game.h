#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>

#define GRID_SIZE 8
#define MAX_SNAKE_LENGTH 64
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
    uint8_t grid[GRID_SIZE][GRID_SIZE];
    Point snake[MAX_SNAKE_LENGTH];
    uint8_t snakeLength;
    Direction direction;
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
void getGridState(uint8_t grid[GRID_SIZE][GRID_SIZE]);
void setLCD();

#endif
