#include "tetris_game.h"
#include <LiquidCrystal.h>

extern LiquidCrystal lcd;

static TetrisState ts;

// PIECES[type][rotation][cell] = {dx, dy} offsets from piece origin
static const int8_t PIECES[7][4][4][2] = {
    // 0: I - Cyan
    {{{0,0},{1,0},{2,0},{3,0}}, {{0,0},{0,1},{0,2},{0,3}},
     {{0,0},{1,0},{2,0},{3,0}}, {{0,0},{0,1},{0,2},{0,3}}},
    // 1: O - Yellow
    {{{0,0},{1,0},{0,1},{1,1}}, {{0,0},{1,0},{0,1},{1,1}},
     {{0,0},{1,0},{0,1},{1,1}}, {{0,0},{1,0},{0,1},{1,1}}},
    // 2: T - Purple
    {{{1,0},{0,1},{1,1},{2,1}}, {{0,0},{0,1},{1,1},{0,2}},
     {{0,0},{1,0},{2,0},{1,1}}, {{1,0},{0,1},{1,1},{1,2}}},
    // 3: S - Green
    {{{1,0},{2,0},{0,1},{1,1}}, {{0,0},{0,1},{1,1},{1,2}},
     {{1,0},{2,0},{0,1},{1,1}}, {{0,0},{0,1},{1,1},{1,2}}},
    // 4: Z - Red
    {{{0,0},{1,0},{1,1},{2,1}}, {{1,0},{0,1},{1,1},{0,2}},
     {{0,0},{1,0},{1,1},{2,1}}, {{1,0},{0,1},{1,1},{0,2}}},
    // 5: J - Blue
    {{{0,0},{0,1},{1,1},{2,1}}, {{0,0},{1,0},{0,1},{0,2}},
     {{0,0},{1,0},{2,0},{2,1}}, {{1,0},{1,1},{0,2},{1,2}}},
    // 6: L - Orange
    {{{2,0},{0,1},{1,1},{2,1}}, {{0,0},{0,1},{0,2},{1,2}},
     {{0,0},{1,0},{2,0},{0,1}}, {{0,0},{1,0},{1,1},{1,2}}},
};

static bool isValid(const TetrisPiece& p) {
    for (int i = 0; i < 4; i++) {
        int nx = p.x + PIECES[p.type][p.rotation][i][0];
        int ny = p.y + PIECES[p.type][p.rotation][i][1];
        if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) return false;
        if (ts.board[ny][nx] != TETRIS_EMPTY) return false;
    }
    return true;
}

static void spawnPiece() {
    ts.current.type     = random(7);
    ts.current.rotation = 0;
    ts.current.x        = 3;
    ts.current.y        = 1;
    if (!isValid(ts.current)) {
        ts.gameOver = true;
    }
}

static void lockPiece() {
    uint8_t val = ts.current.type + 1;
    for (int i = 0; i < 4; i++) {
        int nx = ts.current.x + PIECES[ts.current.type][ts.current.rotation][i][0];
        int ny = ts.current.y + PIECES[ts.current.type][ts.current.rotation][i][1];
        if (nx >= 0 && nx < GRID_W && ny >= 0 && ny < GRID_H) {
            ts.board[ny][nx] = val;
        }
    }
}

static int clearLines() {
    int cleared = 0;
    for (int y = GRID_H - 1; y >= 0; y--) {
        bool full = true;
        for (int x = 0; x < GRID_W; x++) {
            if (ts.board[y][x] == TETRIS_EMPTY) { full = false; break; }
        }
        if (full) {
            for (int ry = y; ry > 0; ry--) {
                for (int x = 0; x < GRID_W; x++) {
                    ts.board[ry][x] = ts.board[ry-1][x];
                }
            }
            for (int x = 0; x < GRID_W; x++) ts.board[0][x] = TETRIS_EMPTY;
            cleared++;
            y++;  // recheck this row
        }
    }
    return cleared;
}

static void awardScore(int lines) {
    static const uint16_t LINE_SCORES[4] = {100, 300, 500, 800};
    if (lines < 1 || lines > 4) return;
    ts.score += LINE_SCORES[lines - 1] * ts.level;
    ts.linesCleared += lines;
    uint8_t newLevel = min((uint8_t)(ts.linesCleared / 10 + 1), (uint8_t)10);
    if (newLevel != ts.level) {
        ts.level = newLevel;
        ts.fallInterval = max(200UL, 800UL - (unsigned long)(ts.level - 1) * 66UL);
    }
}

void initTetris() {
    memset(&ts, 0, sizeof(ts));
    ts.level        = 1;
    ts.fallInterval = 800;
    ts.lastFallTime = millis();

    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lines: 0        ");
    lcd.setCursor(0, 1);
    lcd.print("Score: 0        ");

    spawnPiece();
}

void tetrisLoop() {
    if (ts.gameOver) return;
    unsigned long now = millis();
    if (now - ts.lastFallTime >= ts.fallInterval) {
        ts.lastFallTime = now;
        TetrisPiece next = ts.current;
        next.y++;
        if (isValid(next)) {
            ts.current = next;
        } else {
            lockPiece();
            int lines = clearLines();
            if (lines > 0) awardScore(lines);
            spawnPiece();
        }
    }
}

void tetrisInput(Direction dir) {
    if (ts.gameOver) return;
    TetrisPiece next = ts.current;
    switch (dir) {
        case LEFT:
            next.x--;
            if (isValid(next)) ts.current = next;
            break;
        case RIGHT:
            next.x++;
            if (isValid(next)) ts.current = next;
            break;
        case UP:
            next.rotation = (next.rotation + 1) % 4;
            if (isValid(next)) ts.current = next;
            break;
        case DOWN: {
            // Hard drop
            TetrisPiece drop = ts.current;
            while (true) {
                TetrisPiece tmp = drop;
                tmp.y++;
                if (isValid(tmp)) drop = tmp;
                else break;
            }
            ts.current = drop;
            lockPiece();
            int lines = clearLines();
            if (lines > 0) awardScore(lines);
            spawnPiece();
            break;
        }
    }
}

bool isTetrisGameOver() {
    return ts.gameOver;
}

uint16_t getTetrisScore() {
    return ts.score;
}

void getTetrisGridState(uint8_t out[GRID_H][GRID_W]) {
    memcpy(out, ts.board, sizeof(ts.board));
    if (!ts.gameOver) {
        uint8_t val = ts.current.type + 1;
        for (int i = 0; i < 4; i++) {
            int nx = ts.current.x + PIECES[ts.current.type][ts.current.rotation][i][0];
            int ny = ts.current.y + PIECES[ts.current.type][ts.current.rotation][i][1];
            if (nx >= 0 && nx < GRID_W && ny >= 0 && ny < GRID_H) {
                out[ny][nx] = val;
            }
        }
    }
}

void setTetrisLCD() {
    lcd.setCursor(0, 0);
    lcd.print("Lines: ");
    lcd.print(ts.linesCleared);
    lcd.print("        ");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(ts.score);
    lcd.print("        ");
}
