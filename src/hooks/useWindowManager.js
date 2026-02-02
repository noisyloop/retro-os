import { useState, useCallback } from 'react';

export function useWindowManager() {
  const [openWindows, setOpenWindows] = useState([]);
  const [activeWindow, setActiveWindow] = useState(null);

  const openWindow = useCallback((windowConfig) => {
    setOpenWindows((prev) => {
      if (prev.find((w) => w.id === windowConfig.id)) {
        return prev;
      }
      return [
        ...prev,
        {
          ...windowConfig,
          x: 50 + prev.length * 30,
          y: 50 + prev.length * 30,
        },
      ];
    });
    setActiveWindow(windowConfig.id);
  }, []);

  const closeWindow = useCallback((id) => {
    setOpenWindows((prev) => prev.filter((w) => w.id !== id));
    setActiveWindow((prev) => (prev === id ? null : prev));
  }, []);

  const focusWindow = useCallback((id) => {
    setActiveWindow(id);
  }, []);

  const updateWindowPosition = useCallback((id, x, y) => {
    setOpenWindows((prev) =>
      prev.map((w) => (w.id === id ? { ...w, x, y } : w))
    );
  }, []);

  return {
    openWindows,
    activeWindow,
    openWindow,
    closeWindow,
    focusWindow,
    updateWindowPosition,
  };
}
