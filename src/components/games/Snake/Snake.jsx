import React, { useState, useEffect, useRef } from 'react';
import { WIN95_BUTTON } from '../../../constants';

const GRID_SIZE = 20;
const CELL_SIZE = 15;
const CANVAS_SIZE = GRID_SIZE * CELL_SIZE;
const GAME_SPEED = 150;

export default function Snake() {
  const canvasRef = useRef(null);
  const [gameOver, setGameOver] = useState(false);
  const [score, setScore] = useState(0);
  const [gameStarted, setGameStarted] = useState(false);

  const gameState = useRef({
    snake: [{ x: 10, y: 10 }],
    food: { x: 15, y: 15 },
    direction: { x: 1, y: 0 },
    nextDirection: { x: 1, y: 0 },
  });

  const resetGame = () => {
    gameState.current = {
      snake: [{ x: 10, y: 10 }],
      food: {
        x: Math.floor(Math.random() * GRID_SIZE),
        y: Math.floor(Math.random() * GRID_SIZE),
      },
      direction: { x: 1, y: 0 },
      nextDirection: { x: 1, y: 0 },
    };
    setScore(0);
    setGameOver(false);
    setGameStarted(true);
  };

  useEffect(() => {
    if (!gameStarted || gameOver) return;

    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');

    const gameLoop = setInterval(() => {
      const state = gameState.current;
      state.direction = state.nextDirection;

      const newHead = {
        x: (state.snake[0].x + state.direction.x + GRID_SIZE) % GRID_SIZE,
        y: (state.snake[0].y + state.direction.y + GRID_SIZE) % GRID_SIZE,
      };

      // Check self-collision
      if (state.snake.some((seg) => seg.x === newHead.x && seg.y === newHead.y)) {
        setGameOver(true);
        return;
      }

      state.snake.unshift(newHead);

      // Check food collision
      if (newHead.x === state.food.x && newHead.y === state.food.y) {
        setScore((s) => s + 10);
        state.food = {
          x: Math.floor(Math.random() * GRID_SIZE),
          y: Math.floor(Math.random() * GRID_SIZE),
        };
      } else {
        state.snake.pop();
      }

      // Draw
      ctx.fillStyle = '#000080';
      ctx.fillRect(0, 0, CANVAS_SIZE, CANVAS_SIZE);

      ctx.fillStyle = '#00ff00';
      state.snake.forEach((seg) => {
        ctx.fillRect(
          seg.x * CELL_SIZE + 1,
          seg.y * CELL_SIZE + 1,
          CELL_SIZE - 2,
          CELL_SIZE - 2
        );
      });

      ctx.fillStyle = '#ff0000';
      ctx.fillRect(
        state.food.x * CELL_SIZE + 2,
        state.food.y * CELL_SIZE + 2,
        CELL_SIZE - 4,
        CELL_SIZE - 4
      );
    }, GAME_SPEED);

    return () => clearInterval(gameLoop);
  }, [gameStarted, gameOver]);

  useEffect(() => {
    const handleKey = (e) => {
      if (!gameStarted) return;

      const state = gameState.current;
      const keyMap = {
        ArrowUp: { x: 0, y: -1 },
        ArrowDown: { x: 0, y: 1 },
        ArrowLeft: { x: -1, y: 0 },
        ArrowRight: { x: 1, y: 0 },
      };

      if (keyMap[e.key]) {
        const newDir = keyMap[e.key];
        // Prevent 180-degree turns
        if (newDir.x !== -state.direction.x || newDir.y !== -state.direction.y) {
          state.nextDirection = newDir;
        }
        e.preventDefault();
      }
    };

    window.addEventListener('keydown', handleKey);
    return () => window.removeEventListener('keydown', handleKey);
  }, [gameStarted]);

  return (
    <div style={styles.container}>
      <div style={styles.score}>Score: {score}</div>
      <canvas
        ref={canvasRef}
        width={CANVAS_SIZE}
        height={CANVAS_SIZE}
        style={styles.canvas}
      />
      {(!gameStarted || gameOver) && (
        <div style={styles.controls}>
          {gameOver && <div style={styles.gameOver}>Game Over!</div>}
          <button onClick={resetGame} style={WIN95_BUTTON}>
            {gameOver ? 'Play Again' : 'Start Game'}
          </button>
        </div>
      )}
      <div style={styles.instructions}>Use Arrow Keys to move</div>
    </div>
  );
}

const styles = {
  container: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    padding: '10px',
    background: '#c0c0c0',
  },
  score: {
    marginBottom: '10px',
    fontFamily: 'MS Sans Serif, Arial',
    fontSize: '14px',
  },
  canvas: {
    border: '2px inset #808080',
  },
  controls: {
    marginTop: '10px',
  },
  gameOver: {
    color: 'red',
    marginBottom: '5px',
  },
  instructions: {
    marginTop: '10px',
    fontSize: '12px',
    color: '#404040',
  },
};
