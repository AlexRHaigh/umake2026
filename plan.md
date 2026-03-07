# Snake Game + Web Server Plan

## Overview
Create an 8x8 grid snake game on ESP8266 (ESP-01 1M) with a web server that displays the game status in real-time.

## Configuration Summary
| Setting | Value |
|---------|-------|
| WiFi Mode | Access Point (ESP creates network) |
| Snake AI | Chase food (pathfinding) |
| Game Speed | 500ms per move |
| Web Refresh | 250ms polling |
| Game Over | Wait for restart button |
| Initial Length | 2 cells |
| Score | +1 per food eaten |

---

## Part 1: Snake Game Creation

### Step 1.1: Define Game Data Structures
- [ ] Create enum for cell states: `EMPTY`, `SNAKE_HEAD`, `SNAKE_BODY`, `FOOD`
- [ ] Create enum for direction: `UP`, `DOWN`, `LEFT`, `RIGHT`
- [ ] Create struct `Point` with x, y coordinates
- [ ] Define snake as array of Points (max length 64 for 8x8 grid)
- [ ] Game state struct: grid[8][8], snake[], snakeLength, direction, foodPos, score, gameOver

### Step 1.2: Initialize Game State
- [ ] Clear 8x8 grid to EMPTY
- [ ] Place snake head at position (4, 4)
- [ ] Place snake body at position (4, 5) - initial length of 2
- [ ] Set initial direction to UP
- [ ] Spawn food at random empty cell
- [ ] Set score to 0, gameOver to false

### Step 1.3: Implement Core Game Logic
- [ ] `moveSnake()`: Move all segments, head moves in current direction
- [ ] `checkWallCollision()`: Return true if head hits grid boundary
- [ ] `checkSelfCollision()`: Return true if head hits any body segment
- [ ] `checkFoodEaten()`: Return true if head position equals food position
- [ ] `growSnake()`: Increase snake length by 1, increment score
- [ ] `spawnFood()`: Place food at random empty cell using simple RNG
- [ ] `updateGrid()`: Clear grid, place snake segments and food

### Step 1.4: Implement Food-Chasing AI
- [ ] `calculateDirection()`: Determine best direction to move toward food
- [ ] Use Manhattan distance: prefer direction that reduces distance to food
- [ ] Avoid directions that would cause immediate collision (wall or self)
- [ ] If all directions blocked, pick any valid direction (or game over)

### Step 1.5: Implement Game Loop
- [ ] Track last move time using `millis()`
- [ ] Every 500ms: calculate AI direction, move snake, check collisions
- [ ] On food eaten: grow snake, spawn new food, increment score
- [ ] On collision: set gameOver flag to true, stop movement
- [ ] `resetGame()`: Reinitialize all state for restart

---

## Part 2: Web Server Creation

### Step 2.1: Access Point Setup
- [ ] Configure ESP8266 in AP mode
- [ ] Set SSID: `SnakeGame` (or configurable)
- [ ] Set password: `snake123` (or configurable)
- [ ] IP will be `192.168.4.1` (default AP IP)
- [ ] Print connection info to Serial

### Step 2.2: Create Web Server
- [ ] Initialize `ESP8266WebServer` on port 80
- [ ] Register route handlers
- [ ] Start server in `setup()`
- [ ] Call `server.handleClient()` in `loop()`

### Step 2.3: Define API Endpoints
- [ ] `GET /` - Serve HTML page (stored as PROGMEM string)
- [ ] `GET /status` - Return JSON:
  ```json
  {
    "grid": [[0,0,0,...], ...],
    "score": 0,
    "gameOver": false
  }
  ```
- [ ] `POST /restart` - Reset game and return success

---

## Part 3: Web Display Creation

### Step 3.1: Create HTML Structure
- [ ] Basic HTML5 document
- [ ] 8x8 grid container using CSS Grid
- [ ] 64 div cells for the game board
- [ ] Score display element
- [ ] Restart button (hidden during play, shown on game over)

### Step 3.2: CSS Styling
- [ ] Grid layout: 8 columns, equal cell sizes
- [ ] Cell colors:
  - Empty: dark gray `#333`
  - Snake Head: bright green `#0f0`
  - Snake Body: green `#0a0`
  - Food: red `#f00`
- [ ] Responsive sizing for mobile viewing
- [ ] Restart button styling

### Step 3.3: JavaScript Logic
- [ ] `fetchStatus()`: GET `/status`, parse JSON
- [ ] `updateDisplay()`: Loop through grid array, update cell colors
- [ ] `setInterval()` at 250ms to poll and update
- [ ] Show/hide restart button based on `gameOver` state
- [ ] `restartGame()`: POST to `/restart`, resume polling

---

## File Structure

```
src/
├── main.cpp          # Setup/loop, ties everything together
├── snake_game.h      # Game structs, function declarations
├── snake_game.cpp    # Game logic implementation
├── web_server.h      # Server setup, route handlers
├── web_server.cpp    # Server implementation
└── web_page.h        # HTML/CSS/JS as PROGMEM string
```

---

## Dependencies (platformio.ini)

```ini
lib_deps =
    ESP8266WiFi
    ESP8266WebServer
```

---

## Implementation Order

1. **Step 1.1-1.3**: Basic snake game logic (no AI yet)
2. **Step 1.4**: Add food-chasing AI
3. **Step 1.5**: Game loop with timing
4. **Step 2.1-2.3**: Web server setup
5. **Step 3.1-3.3**: Web display
6. **Integration & Testing**: Connect all pieces, test on device
