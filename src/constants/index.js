// Windows 95 style constants

export const WIN95_COLORS = {
  desktop: '#008080',
  window: '#c0c0c0',
  windowDark: '#808080',
  windowLight: '#dfdfdf',
  titleBarActive: 'linear-gradient(90deg, #000080, #1084d0)',
  titleBarInactive: 'linear-gradient(90deg, #808080, #a0a0a0)',
  text: '#000',
  textLight: '#fff',
  textMuted: '#404040',
};

export const WIN95_BUTTON = {
  padding: '4px 16px',
  background: WIN95_COLORS.window,
  border: `2px outset ${WIN95_COLORS.windowLight}`,
  fontFamily: 'MS Sans Serif, Arial',
  fontSize: '12px',
  cursor: 'pointer',
  outline: 'none',
};

export const WIN95_BUTTON_PRESSED = {
  ...WIN95_BUTTON,
  border: `2px inset ${WIN95_COLORS.windowDark}`,
  background: '#a0a0a0',
};

export const GAMES_CONFIG = [
  { id: 'snake', name: 'Snake', icon: '🐍' },
  { id: 'minesweeper', name: 'Minesweeper', icon: '💣' },
  { id: 'tetris', name: 'Tetris', icon: '🧱' },
  { id: 'memory', name: 'Memory', icon: '🃏' },
  { id: 'pong', name: 'Pong', icon: '🏓' },
];
