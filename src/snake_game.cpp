#include "snake_game.h"

GameState game;

static void spawnFood();
static void updateGrid();
static bool checkWallCollision(Point p);
static bool checkSelfCollision(Point p);
static Direction calculateBestDirection();
static bool isValidMove(Direction dir);
static Point getNextPosition(Direction dir);

void initGame() {
    game.snakeLength = 2;
    game.snake[0] = {4, 4};  // Head
    game.snake[1] = {4, 5};  // Body
    game.direction = UP;
    game.score = 0;
    game.gameOver = false;
    game.lastMoveTime = millis();

    // Clear grid
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            game.grid[y][x] = EMPTY;
        }
    }

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

void getGridState(uint8_t grid[GRID_SIZE][GRID_SIZE]) {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            grid[y][x] = game.grid[y][x];
        }
    }
}

static void spawnFood() {
    // Count empty cells
    int emptyCells = 0;
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (game.grid[y][x] == EMPTY) {
                emptyCells++;
            }
        }
    }

    if (emptyCells == 0) return;

    // Pick random empty cell
    int target = random(emptyCells);
    int count = 0;

    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
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
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            game.grid[y][x] = EMPTY;
        }
    }

    // Place snake
    for (int i = 0; i < game.snakeLength; i++) {
        Point p = game.snake[i];
        if (p.x >= 0 && p.x < GRID_SIZE && p.y >= 0 && p.y < GRID_SIZE) {
            game.grid[p.y][p.x] = (i == 0) ? SNAKE_HEAD : SNAKE_BODY;
        }
    }

    // Place food
    if (game.food.x >= 0 && game.food.x < GRID_SIZE &&
        game.food.y >= 0 && game.food.y < GRID_SIZE) {
        game.grid[game.food.y][game.food.x] = FOOD;
    }
}

static bool checkWallCollision(Point p) {
    return p.x < 0 || p.x >= GRID_SIZE || p.y < 0 || p.y >= GRID_SIZE;
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

    // Update grid and spawn new food if eaten
    updateGrid();

    if (ateFood) {
        spawnFood();
    }
}

void setDirection(Direction dir) {
    // Prevent 180-degree reversal
    if (dir == UP    && game.direction == DOWN)  return;
    if (dir == DOWN  && game.direction == UP)    return;
    if (dir == LEFT  && game.direction == RIGHT) return;
    if (dir == RIGHT && game.direction == LEFT)  return;
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
