import React, { useState, useEffect, useRef } from 'react';
import { WIN95_BUTTON } from '../../../constants';

const COLS = 10;
const ROWS = 20;
const CELL_SIZE = 20;

const SHAPES = [
  [[1, 1, 1, 1]], // I
  [
    [1, 1],
    [1, 1],
  ], // O
  [
    [0, 1, 0],
    [1, 1, 1],
  ], // T
  [
    [1, 0, 0],
    [1, 1, 1],
  ], // L
  [
    [0, 0, 1],
    [1, 1, 1],
  ], // J
  [
    [0, 1, 1],
    [1, 1, 0],
  ], // S
  [
    [1, 1, 0],
    [0, 1, 1],
  ], // Z
];

const COLORS = [
  '#00ffff',
  '#ffff00',
  '#aa00ff',
  '#ff8800',
  '#0000ff',
  '#00ff00',
  '#ff0000',
];

export default function Tetris() {
  const canvasRef = useRef(null);
  const [score, setScore] = useState(0);
  const [gameOver, setGameOver] = useState(false);
  const [gameStarted, setGameStarted] = useState(false);
  const gameRef = useRef(null);

  const spawnPiece = () => {
    const idx = Math.floor(Math.random() * SHAPES.length);
    gameRef.current.piece = SHAPES[idx].map((row) => [...row]);
    gameRef.current.pieceColor = idx;
    gameRef.current.pieceX = 3;
    gameRef.current.pieceY = 0;

    if (collides(0, 0)) {
      setGameOver(true);
      setGameStarted(false);
    }
  };

  const collides = (dx, dy, testPiece = null) => {
    const game = gameRef.current;
    const piece = testPiece || game.piece;

    for (let y = 0; y < piece.length; y++) {
      for (let x = 0; x < piece[y].length; x++) {
        if (piece[y][x]) {
          const newX = game.pieceX + x + dx;
          const newY = game.pieceY + y + dy;

          if (newX < 0 || newX >= COLS || newY >= ROWS) return true;
          if (newY >= 0 && game.board[newY][newX]) return true;
        }
      }
    }
    return false;
  };

  const merge = () => {
    const game = gameRef.current;

    for (let y = 0; y < game.piece.length; y++) {
      for (let x = 0; x < game.piece[y].length; x++) {
        if (game.piece[y][x] && game.pieceY + y >= 0) {
          game.board[game.pieceY + y][game.pieceX + x] = game.pieceColor + 1;
        }
      }
    }

    // Clear completed lines
    let cleared = 0;
    for (let y = ROWS - 1; y >= 0; y--) {
      if (game.board[y].every((c) => c)) {
        game.board.splice(y, 1);
        game.board.unshift(Array(COLS).fill(0));
        cleared++;
        y++;
      }
    }

    if (cleared) {
      setScore((s) => s + cleared * 100);
    }

    spawnPiece();
  };

  const move = (dx, dy) => {
    if (!collides(dx, dy)) {
      gameRef.current.pieceX += dx;
      gameRef.current.pieceY += dy;
      return true;
    }
    return false;
  };

  const rotate = () => {
    const game = gameRef.current;
    const rotated = game.piece[0].map((_, i) =>
      game.piece.map((row) => row[i]).reverse()
    );

    if (!collides(0, 0, rotated)) {
      game.piece = rotated;
    }
  };

  const drop = () => {
    if (!move(0, 1)) {
      merge();
    }
  };

  const startGame = () => {
    gameRef.current = {
      board: Array(ROWS)
        .fill(null)
        .map(() => Array(COLS).fill(0)),
      piece: null,
      pieceX: 0,
      pieceY: 0,
      pieceColor: 0,
    };
    setScore(0);
    setGameOver(false);
    setGameStarted(true);
    spawnPiece();
  };

  // Game loop - drop piece
  useEffect(() => {
    if (!gameStarted || gameOver) return;
    const interval = setInterval(drop, 500);
    return () => clearInterval(interval);
  }, [gameStarted, gameOver]);

  // Render loop
  useEffect(() => {
    if (!gameStarted || gameOver) return;

    const draw = () => {
      const canvas = canvasRef.current;
      const ctx = canvas.getContext('2d');
      const game = gameRef.current;

      // Clear
      ctx.fillStyle = '#000';
      ctx.fillRect(0, 0, COLS * CELL_SIZE, ROWS * CELL_SIZE);

      // Draw board
      for (let y = 0; y < ROWS; y++) {
        for (let x = 0; x < COLS; x++) {
          if (game.board[y][x]) {
            ctx.fillStyle = COLORS[game.board[y][x] - 1];
            ctx.fillRect(
              x * CELL_SIZE + 1,
              y * CELL_SIZE + 1,
              CELL_SIZE - 2,
              CELL_SIZE - 2
            );
          }
        }
      }

      // Draw current piece
      if (game.piece) {
        ctx.fillStyle = COLORS[game.pieceColor];
        for (let y = 0; y < game.piece.length; y++) {
          for (let x = 0; x < game.piece[y].length; x++) {
            if (game.piece[y][x]) {
              ctx.fillRect(
                (game.pieceX + x) * CELL_SIZE + 1,
                (game.pieceY + y) * CELL_SIZE + 1,
                CELL_SIZE - 2,
                CELL_SIZE - 2
              );
            }
          }
        }
      }
    };

    const animFrame = setInterval(draw, 50);
    return () => clearInterval(animFrame);
  }, [gameStarted, gameOver]);

  // Keyboard controls
  useEffect(() => {
    if (!gameStarted) return;

    const handleKey = (e) => {
      switch (e.key) {
        case 'ArrowLeft':
          move(-1, 0);
          break;
        case 'ArrowRight':
          move(1, 0);
          break;
        case 'ArrowDown':
          drop();
          break;
        case 'ArrowUp':
          rotate();
          break;
        default:
          return;
      }
      e.preventDefault();
    };

    window.addEventListener('keydown', handleKey);
    return () => window.removeEventListener('keydown', handleKey);
  }, [gameStarted]);

  return (
    <div style={styles.container}>
      <div style={styles.score}>Score: {score}</div>
      <canvas
        ref={canvasRef}
        width={COLS * CELL_SIZE}
        height={ROWS * CELL_SIZE}
        style={styles.canvas}
      />
      {(!gameStarted || gameOver) && (
        <div style={styles.controls}>
          {gameOver && <div style={styles.gameOver}>Game Over!</div>}
          <button onClick={startGame} style={WIN95_BUTTON}>
            {gameOver ? 'Play Again' : 'Start Game'}
          </button>
        </div>
      )}
      <div style={styles.instructions}>← → Move | ↑ Rotate | ↓ Drop</div>
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
  },
  canvas: {
    border: '3px inset #808080',
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
    fontSize: '11px',
    color: '#404040',
    textAlign: 'center',
  },
};
