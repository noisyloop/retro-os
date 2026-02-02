import React from 'react';

export default function Desktop({ items, onItemDoubleClick }) {
  return (
    <div style={styles.container}>
      {items.map((item) => (
        <DesktopIcon
          key={item.id}
          icon={item.icon}
          name={item.name}
          onDoubleClick={() => onItemDoubleClick(item)}
        />
      ))}
    </div>
  );
}

function DesktopIcon({ icon, name, onDoubleClick }) {
  const [isHovered, setIsHovered] = React.useState(false);

  return (
    <div
      onDoubleClick={onDoubleClick}
      onMouseEnter={() => setIsHovered(true)}
      onMouseLeave={() => setIsHovered(false)}
      style={{
        ...styles.icon,
        background: isHovered ? 'rgba(0,0,128,0.3)' : 'transparent',
      }}
    >
      <div style={styles.iconEmoji}>{icon}</div>
      <span style={styles.iconLabel}>{name}</span>
    </div>
  );
}

const styles = {
  container: {
    padding: '20px',
    display: 'flex',
    flexDirection: 'column',
    gap: '15px',
    position: 'absolute',
    top: 0,
    left: 0,
  },
  icon: {
    width: '75px',
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    cursor: 'pointer',
    padding: '5px',
    borderRadius: '2px',
  },
  iconEmoji: {
    fontSize: '32px',
    textShadow: '2px 2px 2px rgba(0,0,0,0.3)',
  },
  iconLabel: {
    color: '#fff',
    fontSize: '11px',
    textAlign: 'center',
    textShadow: '1px 1px 1px #000',
    marginTop: '4px',
  },
};
