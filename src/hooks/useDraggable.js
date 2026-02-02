import { useState, useCallback } from 'react';

export function useDraggable(onPositionUpdate) {
  const [dragging, setDragging] = useState(null);
  const [dragOffset, setDragOffset] = useState({ x: 0, y: 0 });

  const startDrag = useCallback((e, windowId, windowX, windowY) => {
    setDragging(windowId);
    setDragOffset({
      x: e.clientX - windowX,
      y: e.clientY - windowY,
    });
  }, []);

  const onDrag = useCallback(
    (e) => {
      if (!dragging) return;
      onPositionUpdate(dragging, e.clientX - dragOffset.x, e.clientY - dragOffset.y);
    },
    [dragging, dragOffset, onPositionUpdate]
  );

  const stopDrag = useCallback(() => {
    setDragging(null);
  }, []);

  return {
    dragging,
    startDrag,
    onDrag,
    stopDrag,
  };
}
