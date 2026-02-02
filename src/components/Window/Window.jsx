import React from 'react';
import { WIN95_COLORS } from '../../constants';

export default function Window({
  id,
  title,
  icon,
  x,
  y,
  isActive,
  onClose,
  onFocus,
  onDragStart,
  children,
}) {
  const handleMouseDown = (e) => {
    onDragStart(e, id, x, y);
    onFocus(id);
  };

  const handleClose = (e) => {
    e.stopPropagation();
    onClose(id);
  };

  return (
    <div
      style={{
        ...styles.window,
        left: x,
        top: y,
        boxShadow: isActive
          ? '4px 4px 8px rgba(0,0,0,0.4)'
          : '2px 2px 4px rgba(0,0,0,0.2)',
        zIndex: isActive ? 100 : 10,
      }}
      onClick={() => onFocus(id)}
    >
      {/* Title Bar */}
      <div
        onMouseDown={handleMouseDown}
        style={{
          ...styles.titleBar,
          background: isActive
            ? WIN95_COLORS.titleBarActive
            : WIN95_COLORS.titleBarInactive,
        }}
      >
        <span style={styles.titleText}>
          <span style={styles.titleIcon}>{icon}</span>
          {title}
        </span>
        <div style={styles.windowControls}>
          <button onClick={handleClose} style={styles.closeButton}>
            ✕
          </button>
        </div>
      </div>

      {/* Content */}
      <div style={styles.content}>{children}</div>
    </div>
  );
}

const styles = {
  window: {
    position: 'absolute',
    background: WIN95_COLORS.window,
    border: `2px outset ${WIN95_COLORS.windowLight}`,
  },
  titleBar: {
    padding: '3px 5px',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    cursor: 'move',
  },
  titleText: {
    color: WIN95_COLORS.textLight,
    fontSize: '12px',
    fontWeight: 'bold',
    display: 'flex',
    alignItems: 'center',
    gap: '5px',
  },
  titleIcon: {
    fontSize: '14px',
  },
  windowControls: {
    display: 'flex',
    gap: '2px',
  },
  closeButton: {
    width: '20px',
    height: '20px',
    background: WIN95_COLORS.window,
    border: `2px outset ${WIN95_COLORS.windowLight}`,
    fontSize: '12px',
    fontWeight: 'bold',
    cursor: 'pointer',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    lineHeight: 1,
  },
  content: {
    border: `2px inset ${WIN95_COLORS.windowDark}`,
  },
};
