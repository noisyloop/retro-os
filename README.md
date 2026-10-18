# RetroOS

A nostalgic Windows 95-inspired web operating system built with React, featuring classic arcade games in a retro desktop environment. Initially built on React, but you can now run it on Windows and MacOS as executable files.

## Overview

RetroOS recreates the iconic Windows 95 interface in your browser, complete with draggable windows, a start menu, taskbar, and desktop icons. It includes five classic games that run in separate windows, bringing back the nostalgia of 90s computing.

## Features

- **Authentic Windows 95 UI**: Pixel-perfect recreation of the classic Windows 95 interface with teal desktop background, gray windows, and beveled buttons
- **Draggable Windows**: Click and drag window title bars to reposition them on the desktop
- **Start Menu**: Access all games through the iconic Start button
- **Taskbar**: Shows open windows with active window highlighting and system tray with clock
- **Desktop Icons**: Double-click icons to launch games
- **Multiple Windows**: Run multiple games simultaneously in separate windows

## Games Included

### Snake
Classic snake game where you control a growing serpent. Use arrow keys to navigate and eat food while avoiding collisions with yourself.

**Controls**: Arrow keys to move

### Minesweeper
The timeless logic puzzle game. Click to reveal cells and right-click to flag suspected mines. Clear all safe cells without detonating any mines.

**Controls**: Left-click to reveal, right-click to flag

### Tetris
Arrange falling blocks to create complete lines. As lines are cleared, the game speeds up for increasing challenge.

**Controls**: Arrow keys (left/right to move, up to rotate, down to drop)

### Memory
Match pairs of cards by remembering their positions. Flip two cards at a time to find matching pairs with the fewest moves possible.

**Controls**: Click cards to flip

### Pong
Classic arcade tennis game. Control your paddle to hit the ball past the CPU opponent.

**Controls**: Mouse movement

## Tech Stack

- **React** 19.2.0 - UI library
- **Vite** 7.2.4 - Build tool and dev server
- **JavaScript** - Core language (no TypeScript)
- **HTML5 Canvas** - Game rendering for Snake, Tetris, and Pong
- **CSS-in-JS** - Inline styling for Windows 95 aesthetic

## Prerequisites

- Node.js (v16 or higher recommended)
- npm or yarn package manager

## Installation

1. Clone the repository:
```bash
git clone https://github.com/m0rs3c0d3/retro-os.git
cd retro-os
```

2. Install dependencies:
```bash
npm install
```

## Development

Start the development server:
```bash
npm run dev
```

The application will be available at `http://localhost:5173`

## Build

Create a production build:
```bash
npm run build
```

The built files will be in the `dist` directory.

## Preview Production Build

Preview the production build locally:
```bash
npm run preview
```

## Desktop App (Windows .exe / macOS .dmg)

RetroOS can be packaged as a standalone desktop app with [Electron](https://www.electronjs.org/) and [electron-builder](https://www.electron.build/).

Run it as a desktop app locally:
```bash
npm run electron
```

Build installers (output goes to `release/`):
```bash
npm run dist:win   # RetroOS-<version>-setup.exe and RetroOS-<version>-portable.exe
npm run dist:mac   # RetroOS-<version>-mac-x64.dmg and RetroOS-<version>-mac-arm64.dmg
```

A `.dmg` can only be built on macOS. The Windows installer can be built on Windows, or on Linux/macOS with Wine installed (the portable `.exe` builds without Wine).

### Automated builds

The `Build desktop apps` GitHub Actions workflow builds the `.exe` files on Windows and the `.dmg` files on macOS. Run it manually from the Actions tab to get the files as workflow artifacts, or push a version tag to attach them to a GitHub release:
```bash
git tag v1.0.0
git push origin v1.0.0
```

The builds are not code-signed. On Windows, SmartScreen may warn on first launch ("More info" → "Run anyway"). On macOS, right-click the app and choose "Open" the first time, or run `xattr -cr /Applications/RetroOS.app`.

## Linting

Run ESLint to check code quality:
```bash
npm run lint
```

## Project Structure

```
retro-os/
├── build/           # App icon used for desktop builds
├── electron/        # Electron main process (desktop app)
├── public/          # Static assets
├── src/
│   ├── App.jsx      # Main RetroOS component with all games
│   ├── App.css      # Application styles
│   ├── main.jsx     # React entry point
│   └── index.css    # Global styles
├── index.html       # HTML entry point
├── package.json     # Project dependencies
└── vite.config.js   # Vite configuration
```

## Browser Compatibility

RetroOS works best in modern browsers with support for:
- ES6+ JavaScript features
- HTML5 Canvas API
- CSS Grid and Flexbox
- Mouse and keyboard events

Tested and working on:
- Chrome/Edge (latest)
- Firefox (latest)
- Safari (latest)

## Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest new retro games
- Improve the Windows 95 aesthetic
- Add new features

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by Windows 95 and classic arcade games
- Built with modern web technologies for nostalgic experiences
- Created for educational and entertainment purposes

## Future Ideas

- Additional classic games (Breakout, Space Invaders, Pac-Man clone)
- Notepad application
- Paint application
- Calculator
- File explorer simulation
- Multiple desktop themes
- Sound effects and background music
- High score persistence with localStorage
- Customizable window colors

---

Built with React and nostalgia.
