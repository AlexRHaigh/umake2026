#include "snake_game.h"
#include <LiquidCrystal.h>

GameState game;

static void spawnFood();
static void updateGrid();
static bool checkWallCollision(Point p);
static bool checkSelfCollision(Point p);
static Direction calculateBestDirection();
static bool isValidMove(Direction dir);
static Point getNextPosition(Direction dir);

#define RS 22
#define E  5
#define D4 17
#define D5 21
#define D6 19
#define D7 18

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

static unsigned long gameStartTime = 0;

void initGame() {
    game.snakeLength = 2;
    game.snake[0] = {4, 4};  // Head
    game.snake[1] = {4, 5};  // Body
    game.direction = UP;
    game.lastMoved = UP;
    game.score = 0;
    game.gameOver = false;
    game.lastMoveTime = millis();

    // Clear grid
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            game.grid[y][x] = EMPTY;
        }
    }

    gameStartTime = millis();

    lcd.begin(16, 2);
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("Timer: 0 Sec    ");

    lcd.setCursor(0,1);
    lcd.print("Score: 0        ");

    spawnFood();
    updateGrid();
}

void resetGame() {
    initGame();
}

bool isGameOver() {
    return game.gameOver;
}

uint16_t getScore() {
    return game.score;
}

void getGridState(uint8_t grid[GRID_H][GRID_W]) {
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            grid[y][x] = game.grid[y][x];
        }
    }
}

static void spawnFood() {
    // Count empty cells
    int emptyCells = 0;
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (game.grid[y][x] == EMPTY) {
                emptyCells++;
            }
        }
    }

    if (emptyCells == 0) return;

    // Pick random empty cell
    int target = random(emptyCells);
    int count = 0;

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (game.grid[y][x] == EMPTY) {
                if (count == target) {
                    game.food = {(int8_t)x, (int8_t)y};
                    game.grid[y][x] = FOOD;
                    return;
                }
                count++;
            }
        }
    }
}

static void updateGrid() {
    // Clear grid
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            game.grid[y][x] = EMPTY;
        }
    }

    // Place snake
    for (int i = 0; i < game.snakeLength; i++) {
        Point p = game.snake[i];
        if (p.x >= 0 && p.x < GRID_W && p.y >= 0 && p.y < GRID_H) {
            game.grid[p.y][p.x] = (i == 0) ? SNAKE_HEAD : SNAKE_BODY;
        }
    }

    // Place food
    if (game.food.x >= 0 && game.food.x < GRID_W &&
        game.food.y >= 0 && game.food.y < GRID_H) {
        game.grid[game.food.y][game.food.x] = FOOD;
    }
}

static bool checkWallCollision(Point p) {
    return p.x < 0 || p.x >= GRID_W || p.y < 0 || p.y >= GRID_H;
}

static bool checkSelfCollision(Point p) {
    // Check against body segments (skip head at index 0)
    for (int i = 1; i < game.snakeLength; i++) {
        if (game.snake[i].x == p.x && game.snake[i].y == p.y) {
            return true;
        }
    }
    return false;
}

static Point getNextPosition(Direction dir) {
    Point next = game.snake[0];
    switch (dir) {
        case UP:    next.y--; break;
        case DOWN:  next.y++; break;
        case LEFT:  next.x--; break;
        case RIGHT: next.x++; break;
    }
    // Wrap around edges
    next.x = (next.x + GRID_W) % GRID_W;
    next.y = (next.y + GRID_H) % GRID_H;
    return next;
}

static bool isValidMove(Direction dir) {
    Point next = getNextPosition(dir);

    // Check wall collision
    if (checkWallCollision(next)) return false;

    // Check self collision (excluding tail which will move)
    for (int i = 1; i < game.snakeLength - 1; i++) {
        if (game.snake[i].x == next.x && game.snake[i].y == next.y) {
            return false;
        }
    }

    return true;
}

static int manhattanDistance(Point a, Point b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

static Direction calculateBestDirection() {
    Direction directions[] = {UP, DOWN, LEFT, RIGHT};
    Direction bestDir = game.direction;
    int bestDist = 999;
    bool foundValid = false;

    for (int i = 0; i < 4; i++) {
        Direction dir = directions[i];

        if (!isValidMove(dir)) continue;

        Point next = getNextPosition(dir);
        int dist = manhattanDistance(next, game.food);

        if (!foundValid || dist < bestDist) {
            bestDist = dist;
            bestDir = dir;
            foundValid = true;
        }
    }

    return bestDir;
}

static void moveSnake() {
    game.lastMoved = game.direction;
    Point newHead = getNextPosition(game.direction);

    // Check collisions
    if (checkWallCollision(newHead) || checkSelfCollision(newHead)) {
        game.gameOver = true;
        return;
    }

    // Check if eating food
    bool ateFood = (newHead.x == game.food.x && newHead.y == game.food.y);

    // Shift body segments
    if (!ateFood) {
        // Move each segment to position of segment ahead of it
        for (int i = game.snakeLength - 1; i > 0; i--) {
            game.snake[i] = game.snake[i - 1];
        }
    } else {
        // Grow snake - shift and add new segment
        if (game.snakeLength < MAX_SNAKE_LENGTH) {
            for (int i = game.snakeLength; i > 0; i--) {
                game.snake[i] = game.snake[i - 1];
            }
            game.snakeLength++;
            game.score++;
        }
    }

    // Move head to new position
    game.snake[0] = newHead;

    // Spawn new food first so updateGrid draws it at the new position,
    // not at the cell the snake just moved onto.
    if (ateFood) {
        spawnFood();
    }

    updateGrid();
}

void setDirection(Direction dir) {
    // Check against lastMoved (not queued direction) to prevent chained
    // inputs within one tick from sneaking in a 180-degree reversal.
    if (dir == UP    && game.lastMoved == DOWN)  return;
    if (dir == DOWN  && game.lastMoved == UP)    return;
    if (dir == LEFT  && game.lastMoved == RIGHT) return;
    if (dir == RIGHT && game.lastMoved == LEFT)  return;
    game.direction = dir;
}

void gameLoop() {
    if (game.gameOver) return;
    unsigned long currentTime = millis();

    if (currentTime - game.lastMoveTime >= GAME_SPEED_MS) {
        game.lastMoveTime = currentTime;
        moveSnake();
    }
}

void setLCD() {
    unsigned long elapsedSec = (millis() - gameStartTime) / 1000;

    lcd.setCursor(0, 0);
    lcd.print("Timer: ");
    lcd.print(elapsedSec);
    lcd.print(" Sec    ");

    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(game.score);
    lcd.print("        ");
}
