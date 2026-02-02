import React, { useState } from 'react';
import { WIN95_COLORS } from '../../constants';

export default function StartMenu({ items, onItemClick, onClose }) {
  return (
    <>
      {/* Backdrop to close menu */}
      <div style={styles.backdrop} onClick={onClose} />

      {/* Menu */}
      <div style={styles.menu}>
        {/* Sidebar */}
        <div style={styles.sidebar}>
          <span style={styles.sidebarText}>
            RetroOS<span style={styles.sidebarHighlight}>95</span>
          </span>
        </div>

        {/* Menu Items */}
        <div style={styles.menuContent}>
          <div style={styles.menuHeader}>🎮 Games</div>

          {items.map((item) => (
            <MenuItem
              key={item.id}
              icon={item.icon}
              name={item.name}
              onClick={() => onItemClick(item)}
            />
          ))}

          <div style={styles.divider} />

          <MenuItem icon="🔌" name="Shut Down..." onClick={() => {}} />
        </div>
      </div>
    </>
  );
}

function MenuItem({ icon, name, onClick }) {
  const [isHovered, setIsHovered] = useState(false);

  return (
    <div
      onClick={onClick}
      onMouseEnter={() => setIsHovered(true)}
      onMouseLeave={() => setIsHovered(false)}
      style={{
        ...styles.menuItem,
        background: isHovered ? '#000080' : 'transparent',
        color: isHovered ? '#fff' : '#000',
      }}
    >
      <span style={styles.menuItemIcon}>{icon}</span>
      {name}
    </div>
  );
}

const styles = {
  backdrop: {
    position: 'fixed',
    top: 0,
    left: 0,
    right: 0,
    bottom: '36px',
    zIndex: 999,
  },
  menu: {
    position: 'absolute',
    bottom: '40px',
    left: '0',
    width: '220px',
    background: WIN95_COLORS.window,
    border: `2px outset ${WIN95_COLORS.windowLight}`,
    boxShadow: '4px 4px 8px rgba(0,0,0,0.3)',
    zIndex: 1000,
  },
  sidebar: {
    background: 'linear-gradient(180deg, #000080, #1084d0)',
    padding: '10px 8px',
    writingMode: 'vertical-rl',
    textOrientation: 'mixed',
    transform: 'rotate(180deg)',
    position: 'absolute',
    left: '0',
    top: '0',
    height: '100%',
    width: '30px',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
  },
  sidebarText: {
    color: '#c0c0c0',
    fontSize: '20px',
    fontWeight: 'bold',
    letterSpacing: '2px',
  },
  sidebarHighlight: {
    color: '#fff',
  },
  menuContent: {
    marginLeft: '30px',
    padding: '5px 0',
  },
  menuHeader: {
    padding: '5px 10px',
    borderBottom: `1px solid ${WIN95_COLORS.windowDark}`,
    fontSize: '12px',
    color: WIN95_COLORS.textMuted,
  },
  menuItem: {
    padding: '8px 15px',
    display: 'flex',
    alignItems: 'center',
    gap: '10px',
    cursor: 'pointer',
    fontSize: '13px',
  },
  menuItemIcon: {
    fontSize: '18px',
  },
  divider: {
    borderTop: `2px groove ${WIN95_COLORS.windowLight}`,
    margin: '5px 0',
  },
};
