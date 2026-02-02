import React from 'react';
import { WIN95_BUTTON, WIN95_BUTTON_PRESSED, WIN95_COLORS } from '../../constants';

export default function Taskbar({
  isStartMenuOpen,
  onStartClick,
  openWindows,
  activeWindow,
  onWindowClick,
  time,
}) {
  return (
    <div style={styles.taskbar}>
      {/* Start Button */}
      <button
        onClick={onStartClick}
        style={{
          ...(isStartMenuOpen ? WIN95_BUTTON_PRESSED : WIN95_BUTTON),
          padding: '4px 10px',
          display: 'flex',
          alignItems: 'center',
          gap: '5px',
          fontWeight: 'bold',
        }}
      >
        <span style={{ fontSize: '16px' }}>🪟</span>
        Start
      </button>

      {/* Separator */}
      <div style={styles.separator} />

      {/* Open Windows */}
      <div style={styles.windowButtons}>
        {openWindows.map((win) => (
          <button
            key={win.id}
            onClick={() => onWindowClick(win.id)}
            style={{
              ...(activeWindow === win.id ? WIN95_BUTTON_PRESSED : WIN95_BUTTON),
              ...styles.windowButton,
            }}
          >
            <span style={{ fontSize: '14px' }}>{win.icon}</span>
            <span style={styles.windowButtonText}>{win.name}</span>
          </button>
        ))}
      </div>

      {/* System Tray */}
      <div style={styles.systemTray}>
        <span>🔊</span>
        <span>{time}</span>
      </div>
    </div>
  );
}

const styles = {
  taskbar: {
    position: 'absolute',
    bottom: 0,
    left: 0,
    right: 0,
    height: '36px',
    background: WIN95_COLORS.window,
    borderTop: `2px outset ${WIN95_COLORS.windowLight}`,
    display: 'flex',
    alignItems: 'center',
    padding: '0 4px',
    gap: '4px',
  },
  separator: {
    width: '2px',
    height: '24px',
    background: WIN95_COLORS.windowDark,
    marginLeft: '4px',
  },
  windowButtons: {
    flex: 1,
    display: 'flex',
    gap: '4px',
    overflow: 'hidden',
  },
  windowButton: {
    minWidth: '120px',
    maxWidth: '160px',
    display: 'flex',
    alignItems: 'center',
    gap: '5px',
    textAlign: 'left',
    overflow: 'hidden',
  },
  windowButtonText: {
    overflow: 'hidden',
    textOverflow: 'ellipsis',
    whiteSpace: 'nowrap',
  },
  systemTray: {
    padding: '2px 8px',
    border: `2px inset ${WIN95_COLORS.windowDark}`,
    background: WIN95_COLORS.window,
    fontSize: '12px',
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
  },
};
