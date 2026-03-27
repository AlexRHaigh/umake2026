# umake2026

An ESP32-based gaming platform featuring Snake and Tetris on an 8x16 WS2812B LED matrix, with a WiFi web spectator interface and wireless controller support via ESP-NOW.

---

## Table of Contents

- [Hardware](#hardware)
- [Project Structure](#project-structure)
- [Games](#games)
- [Web Server](#web-server)
- [Connecting to the Web Server](#connecting-to-the-web-server)
- [Node.js Proxy Server (Audience Voting)](#nodejs-proxy-server-audience-voting)
- [Wireless Controller](#wireless-controller)
- [Building & Flashing](#building--flashing)

---

## Hardware

| Component | Details |
|-----------|---------|
| Microcontroller | ESP32-DevKit |
| LED Matrix | 128x WS2812B RGB LEDs (8 columns × 16 rows), GPIO 23 |
| LCD | 16x2 character display (RS: GPIO 22, E: GPIO 5, D4-D7: GPIOs 17, 21, 19, 18) |
| Wireless | WiFi 802.11 b/g/n + ESP-NOW peer-to-peer |

---

## Project Structure

```
umake2026/
├── src/                  # Main ESP32 firmware
│   ├── main.cpp          # Entry point, game mode manager
│   ├── snake_game.h/.cpp # Snake game logic
│   ├── tetris_game.h/.cpp# Tetris logic
│   ├── led_display.h/.cpp# WS2812B LED driver and animations
│   ├── web_server.h/.cpp # HTTP server and API endpoints
│   └── web_page.h        # Embedded HTML/CSS/JS (~18KB)
├── server/               # Node.js audience voting proxy
│   ├── index.js
│   └── package.json
├── Controller_Code/      # Wireless controller firmware
│   └── Controller.cpp
├── platformio.ini        # PlatformIO build config
└── plan.md               # Original design spec
```

---

## Games

### Snake

- Grid: 8x16
- ESP-NOW directions control the snake
- Special effects can be applied via voting:
  - **SPEED_UP** — 2× game speed for 10 seconds
  - **REVERSE** — inverts controls for 5 seconds
  - **WALLS** — adds obstacles for 10 seconds
  - **SHRINK** (labelled "GROW") — extends snake by 4 segments
- LCD displays elapsed time and score

### Tetris

- Grid: 8x16, 7 standard tetrominos
- UP: rotate, DOWN: hard drop, LEFT/RIGHT: nudge
- Lines cleared increase level and speed

### Game Selection

On boot, the LED matrix shows a selection screen (left = Snake, right = Tetris). Press LEFT on the controller to start Snake, RIGHT for Tetris.

---

## Web Server

The ESP32 runs an HTTP server on **port 80** providing a live spectator view and control API.

### Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | Serves the spectator web interface |
| GET | `/status` | Returns current game state as JSON |
| POST | `/restart` | Resets the current game |
| POST | `/effect` | Applies a named effect |

### `/status` Response

```json
{
  "grid": [[0,0,1,...], ...],
  "score": 12,
  "gameOver": false,
  "effects": "SPEED_UP,WALLS"
}
```

Grid cell values: `0` = empty, `1` = snake head, `2` = snake body, `3` = food, `4` = wall.

---

## Connecting to the Web Server

1. **Flash the firmware** to the ESP32 (see [Building & Flashing](#building--flashing)).


2. **Power on the ESP32** and open the Serial Monitor at **115200 baud**. The device will print its IP address once connected:
   ```
   Connected to WiFi
   IP address: 192.168.X.X
   ```

3. **Open a browser** on any device on the same network and navigate to:
   ```
   http://192.168.X.X
   ```
   You will see the **VIPER-1 LIVE FEED** cyberpunk-styled spectator interface showing the live game grid, score, and game status.

---

## Node.js Proxy Server (Audience Voting)

The `server/` directory contains an Express-based proxy that lets a streaming audience vote on which effect to apply every 5 seconds.

### Setup

```bash
cd server
npm install
ESP_IP=192.168.X.X node index.js
```

Replace `192.168.X.X` with the ESP32's IP address. The server runs on **port 3000**.

### How It Works

1. **Voting phase (5s)**: Two random effects are offered (OVERCLOCK, MIRROR MAZE, SECTOR BLOCK, GROW).
2. **Tally**: Most-voted effect wins (random on tie).
3. **Apply**: Winning effect is POSTed to the ESP32 `/effect` endpoint.
4. **Cooldown (5s)**: Waits before the next vote cycle.

---

## Wireless Controller

The controller is a separate ESP32 running `Controller_Code/Controller.cpp`. It sends button presses to the main unit via **ESP-NOW**.

- **Receiver MAC address** (set in controller code): `70:4B:CA:8E:9F:48`
- Button messages: `"UP"`, `"DOWN"`, `"LEFT"`, `"RIGHT"`

To pair a new controller, update the MAC address in `Controller_Code/Controller.cpp` to match the main ESP32's MAC address.

---

## Building & Flashing

This project uses [PlatformIO](https://platformio.org/).

```bash
# Install PlatformIO CLI if needed
pip install platformio

# Build and flash
pio run --target upload

# Open serial monitor
pio device monitor --baud 115200
```

**Dependencies** (managed by PlatformIO):
- `fastled/FastLED ^3.10.3`
- `arduino-libraries/LiquidCrystal ^1.0.7`

---

*Youssof Was Here ;)*
