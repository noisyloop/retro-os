# RetroOS (C++ / SDL2)

The native version of RetroOS: the same Windows 95-style desktop and five games,
rewritten in C++17 on SDL2, plus a Paint program. No browser or Electron, just one
small binary.

- Draggable windows, close boxes, taskbar with Start menu and clock, desktop icons
- Games: Snake, Minesweeper, Tetris, Memory, Pong
- Paint: pencil, brush, airbrush, eraser, fill, colour picker, line, rectangle,
  ellipse and rounded rectangle, a 28-colour palette, undo/redo and BMP save/open

## Build

Needs `cmake`, `sdl2` and `sdl2_ttf` (Arch/Omarchy: `sudo pacman -S --needed cmake sdl2 sdl2_ttf`).

```sh
cmake -S . -B build && cmake --build build
./build/retroos
```

Double-click a desktop icon or use **Start** to open an app.
`RETRO_AUTOLAUNCH=Paint,Tetris ./build/retroos` opens apps at startup.

## Controls

| App | Controls |
|---|---|
| Snake | Arrows/WASD, P pause, Enter restart |
| Minesweeper | Left-click reveal, right-click flag, click the header to restart |
| Tetris | Left/right move, Up rotate, Down soft drop, Space hard drop, P pause |
| Memory | Click two cards to flip them |
| Pong | Move the mouse; first to 7 |

### Paint

- Left-drag draws in the foreground colour, right-drag in the background colour.
- Palette: left click sets the foreground, right click the background. Click the
  colour preview (or press X) to swap them.
- The box under the toolbox sets the size for Brush, Airbrush, Eraser and Line, and
  the fill style for shapes (outline, outline + background fill, solid). Shapes use
  the Line width for their outline.
- Hold Shift for 45-degree lines, squares and circles. Esc cancels a stroke in progress.
- Tool keys: P pencil, B brush, A airbrush, E eraser, F fill, I pick colour, L line,
  R rectangle, O ellipse, U rounded rectangle.
- Ctrl+Z / Ctrl+Y undo and redo (32 steps), Ctrl+N new picture.
- Ctrl+S saves `retroos-paint-<date>-<time>.bmp` to `~/Pictures` (or `~` if that
  doesn't exist). Drop a `.bmp` onto the RetroOS window to open it; anything larger
  than the 494x346 canvas is cropped.

## Adding an app

Write a class implementing `App` (`src/app.hpp`) in `src/apps/`, then register it
with `RETRO_REGISTER_APP("Name", Type)`. It shows up on the desktop and in the
Start menu, and CMake picks the file up automatically.
