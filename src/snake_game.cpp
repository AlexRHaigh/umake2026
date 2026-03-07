#include "snake_game.h"
#include <LiquidCrystal.h>

GameState game;

static volatile bool g_restartRequested = false;

bool consumeRestartRequest() {
    if (g_restartRequested) {
        g_restartRequested = false;
        return true;
    }
    return false;
}

void requestRestart() {
    g_restartRequested = true;
}

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

    game.reverseControls = false;
    game.reverseUntil = 0;
    game.speedUp = false;
    game.speedUpUntil = 0;
    game.wallsActive = false;
    game.wallsUntil = 0;

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

static void drawWalls() {
    // y=5: x=0..6 (gap at x=7)
    for (int x = 0; x <= 6; x++) game.grid[5][x] = WALL;
    // y=10: x=1..7 (gap at x=0)
    for (int x = 1; x <= 7; x++) game.grid[10][x] = WALL;
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

    // Re-draw walls on top so they persist
    if (game.wallsActive) drawWalls();
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

    // Check WALL cell collision
    if (next.x >= 0 && next.x < GRID_W && next.y >= 0 && next.y < GRID_H &&
        game.grid[next.y][next.x] == WALL) return false;

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

    // Check collisions (wall cells count as fatal)
    bool hitWallCell = (newHead.x >= 0 && newHead.x < GRID_W &&
                        newHead.y >= 0 && newHead.y < GRID_H &&
                        game.grid[newHead.y][newHead.x] == WALL);
    if (checkWallCollision(newHead) || checkSelfCollision(newHead) || hitWallCell) {
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
    if (game.gameOver) {
        if (dir == UP) g_restartRequested = true;
        return;
    }

    // Flip controls if REVERSE effect is active
    if (game.reverseControls) {
        if      (dir == UP)    dir = DOWN;
        else if (dir == DOWN)  dir = UP;
        else if (dir == LEFT)  dir = RIGHT;
        else if (dir == RIGHT) dir = LEFT;
    }

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

    // Expire effects
    if (game.speedUp && currentTime >= game.speedUpUntil) {
        game.speedUp = false;
    }
    if (game.reverseControls && currentTime >= game.reverseUntil) {
        game.reverseControls = false;
    }
    if (game.wallsActive && currentTime >= game.wallsUntil) {
        game.wallsActive = false;
        updateGrid(); // redraw without walls
    }

    unsigned long tickInterval = game.speedUp ? 250 : GAME_SPEED_MS;
    if (currentTime - game.lastMoveTime >= tickInterval) {
        game.lastMoveTime = currentTime;
        moveSnake();
    }
}

void applyEffect(const char* effect) {
    if (strcmp(effect, "SPEED_UP") == 0) {
        game.speedUp = true;
        game.speedUpUntil = millis() + 10000;
    } else if (strcmp(effect, "REVERSE") == 0) {
        game.reverseControls = true;
        game.reverseUntil = millis() + 5000;
    } else if (strcmp(effect, "WALLS") == 0) {
        game.wallsActive = true;
        game.wallsUntil = millis() + 10000;
        drawWalls();
    } else if (strcmp(effect, "SHRINK") == 0) {
        if (game.snakeLength > 1) {
            game.snakeLength = max((int)(game.snakeLength / 2), 1);
        }
        updateGrid();
    }
}

void getActiveEffects(char* buf, size_t bufSize) {
    buf[0] = '\0';
    unsigned long now = millis();
    bool first = true;

    auto append = [&](const char* id) {
        if (!first) strncat(buf, ",", bufSize - strlen(buf) - 1);
        strncat(buf, id, bufSize - strlen(buf) - 1);
        first = false;
    };

    if (game.speedUp) {
        if (now < game.speedUpUntil) append("SPEED_UP");
        else game.speedUp = false;
    }
    if (game.reverseControls) {
        if (now < game.reverseUntil) append("REVERSE");
        else game.reverseControls = false;
    }
    if (game.wallsActive) {
        if (now < game.wallsUntil) append("WALLS");
        else game.wallsActive = false;
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

void initLCD() {
    lcd.begin(16, 2);
    lcd.clear();
}

void setSelectLCD() {
    lcd.setCursor(0, 0);
    lcd.print("L:SNAKE R:TETRIS");
    lcd.setCursor(0, 1);
    lcd.print("Press L or R    ");
}
