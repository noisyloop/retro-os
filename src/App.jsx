import React, { useState } from 'react';
import { Desktop, Window, Taskbar, StartMenu } from './components';
import { Snake, Minesweeper, Tetris, Memory, Pong } from './components/games';
import { useWindowManager, useDraggable, useClock } from './hooks';
import { WIN95_COLORS, GAMES_CONFIG } from './constants';

// Map game IDs to their components
const GAME_COMPONENTS = {
  snake: Snake,
  minesweeper: Minesweeper,
  tetris: Tetris,
  memory: Memory,
  pong: Pong,
};

export default function RetroOS() {
  const [startMenuOpen, setStartMenuOpen] = useState(false);
  const { formattedTime } = useClock();

  const {
    openWindows,
    activeWindow,
    openWindow,
    closeWindow,
    focusWindow,
    updateWindowPosition,
  } = useWindowManager();

  const { startDrag, onDrag, stopDrag } = useDraggable(updateWindowPosition);

  const handleOpenGame = (game) => {
    openWindow(game);
    setStartMenuOpen(false);
  };

  const handleStartClick = () => {
    setStartMenuOpen((prev) => !prev);
  };

  return (
    <div
      style={styles.desktop}
      onMouseMove={onDrag}
      onMouseUp={stopDrag}
    >
      {/* Desktop Icons */}
      <Desktop items={GAMES_CONFIG} onItemDoubleClick={handleOpenGame} />

      {/* Windows */}
      {openWindows.map((win) => {
        const GameComponent = GAME_COMPONENTS[win.id];
        return (
          <Window
            key={win.id}
            id={win.id}
            title={win.name}
            icon={win.icon}
            x={win.x}
            y={win.y}
            isActive={activeWindow === win.id}
            onClose={closeWindow}
            onFocus={focusWindow}
            onDragStart={startDrag}
          >
            <GameComponent />
          </Window>
        );
      })}

      {/* Start Menu */}
      {startMenuOpen && (
        <StartMenu
          items={GAMES_CONFIG}
          onItemClick={handleOpenGame}
          onClose={() => setStartMenuOpen(false)}
        />
      )}

      {/* Taskbar */}
      <Taskbar
        isStartMenuOpen={startMenuOpen}
        onStartClick={handleStartClick}
        openWindows={openWindows}
        activeWindow={activeWindow}
        onWindowClick={focusWindow}
        time={formattedTime}
      />
    </div>
  );
}

const styles = {
  desktop: {
    width: '100vw',
    height: '100vh',
    background: WIN95_COLORS.desktop,
    position: 'relative',
    overflow: 'hidden',
    fontFamily: '"MS Sans Serif", "Segoe UI", Tahoma, sans-serif',
    userSelect: 'none',
  },
};
