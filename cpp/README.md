# RetroOS (C++ / SDL2)

The native version of RetroOS: the same Windows 95-style desktop and five games,
rewritten in C++17 on SDL2. No browser or Electron, just one small binary.

- Draggable windows, close boxes, taskbar with Start menu and clock, desktop icons
- Games: Snake, Minesweeper, Tetris, Memory, Pong

## Build

Needs `cmake`, `sdl2` and `sdl2_ttf` (Arch/Omarchy: `sudo pacman -S --needed cmake sdl2 sdl2_ttf`).

```sh
cmake -S . -B build && cmake --build build
./build/retroos
```

Double-click a desktop icon or use **Start** to open a game.
`RETRO_AUTOLAUNCH=Tetris,Pong ./build/retroos` opens apps at startup.

## Controls

| Game | Controls |
|---|---|
| Snake | Arrows/WASD, P pause, Enter restart |
| Minesweeper | Left-click reveal, right-click flag, click the header to restart |
| Tetris | Left/right move, Up rotate, Down soft drop, Space hard drop, P pause |
| Memory | Click two cards to flip them |
| Pong | Move the mouse; first to 7 |

## Adding an app

Write a class implementing `App` (`src/app.hpp`) in `src/apps/`, then register it
with `RETRO_REGISTER_APP("Name", Type)`. It shows up on the desktop and in the
Start menu, and CMake picks the file up automatically.
