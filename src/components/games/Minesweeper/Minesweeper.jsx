import React, { useState, useEffect, useCallback } from 'react';
import { WIN95_BUTTON } from '../../../constants';

const BOARD_SIZE = 8;
const MINE_COUNT = 10;
const NUMBER_COLORS = [
  '',
  '#0000ff',
  '#008000',
  '#ff0000',
  '#000080',
  '#800000',
  '#008080',
  '#000000',
  '#808080',
];

function createBoard() {
  const board = Array(BOARD_SIZE)
    .fill(null)
    .map(() =>
      Array(BOARD_SIZE)
        .fill(null)
        .map(() => ({
          isMine: false,
          isRevealed: false,
          isFlagged: false,
          neighborMines: 0,
        }))
    );

  // Place mines
  let placed = 0;
  while (placed < MINE_COUNT) {
    const x = Math.floor(Math.random() * BOARD_SIZE);
    const y = Math.floor(Math.random() * BOARD_SIZE);
    if (!board[y][x].isMine) {
      board[y][x].isMine = true;
      placed++;
    }
  }

  // Calculate neighbor counts
  for (let y = 0; y < BOARD_SIZE; y++) {
    for (let x = 0; x < BOARD_SIZE; x++) {
      if (!board[y][x].isMine) {
        let count = 0;
        for (let dy = -1; dy <= 1; dy++) {
          for (let dx = -1; dx <= 1; dx++) {
            const ny = y + dy;
            const nx = x + dx;
            if (
              ny >= 0 &&
              ny < BOARD_SIZE &&
              nx >= 0 &&
              nx < BOARD_SIZE &&
              board[ny][nx].isMine
            ) {
              count++;
            }
          }
        }
        board[y][x].neighborMines = count;
      }
    }
  }

  return board;
}

export default function Minesweeper() {
  const [board, setBoard] = useState([]);
  const [gameOver, setGameOver] = useState(false);
  const [won, setWon] = useState(false);
  const [flagCount, setFlagCount] = useState(0);

  const initBoard = useCallback(() => {
    setBoard(createBoard());
    setGameOver(false);
    setWon(false);
    setFlagCount(0);
  }, []);

  useEffect(() => {
    initBoard();
  }, [initBoard]);

  const reveal = (y, x) => {
    if (gameOver || won || board[y][x].isRevealed || board[y][x].isFlagged) {
      return;
    }

    const newBoard = board.map((row) => row.map((cell) => ({ ...cell })));

    if (newBoard[y][x].isMine) {
      // Game over - reveal all mines
      newBoard.forEach((row) =>
        row.forEach((cell) => {
          if (cell.isMine) cell.isRevealed = true;
        })
      );
      setBoard(newBoard);
      setGameOver(true);
      return;
    }

    // Flood fill for empty cells
    const flood = (cy, cx) => {
      if (cy < 0 || cy >= BOARD_SIZE || cx < 0 || cx >= BOARD_SIZE) return;
      if (
        newBoard[cy][cx].isRevealed ||
        newBoard[cy][cx].isFlagged ||
        newBoard[cy][cx].isMine
      ) {
        return;
      }

      newBoard[cy][cx].isRevealed = true;

      if (newBoard[cy][cx].neighborMines === 0) {
        for (let dy = -1; dy <= 1; dy++) {
          for (let dx = -1; dx <= 1; dx++) {
            flood(cy + dy, cx + dx);
          }
        }
      }
    };

    flood(y, x);
    setBoard(newBoard);

    // Check win condition
    const unrevealed = newBoard
      .flat()
      .filter((c) => !c.isRevealed && !c.isMine).length;
    if (unrevealed === 0) {
      setWon(true);
    }
  };

  const toggleFlag = (e, y, x) => {
    e.preventDefault();
    if (gameOver || won || board[y][x].isRevealed) return;

    const newBoard = board.map((row) => row.map((cell) => ({ ...cell })));
    newBoard[y][x].isFlagged = !newBoard[y][x].isFlagged;
    setBoard(newBoard);
    setFlagCount((f) => (newBoard[y][x].isFlagged ? f + 1 : f - 1));
  };

  const getCellContent = (cell) => {
    if (cell.isFlagged) return '🚩';
    if (!cell.isRevealed) return '';
    if (cell.isMine) return '💣';
    return cell.neighborMines || '';
  };

  return (
    <div style={styles.container}>
      <div style={styles.header}>
        <span>💣 {MINE_COUNT - flagCount}</span>
        <span>{gameOver ? '😵' : won ? '😎' : '🙂'}</span>
        <button onClick={initBoard} style={{ ...WIN95_BUTTON, padding: '2px 8px' }}>
          New
        </button>
      </div>

      <div style={styles.board}>
        {board.map((row, y) =>
          row.map((cell, x) => (
            <div
              key={`${y}-${x}`}
              onClick={() => reveal(y, x)}
              onContextMenu={(e) => toggleFlag(e, y, x)}
              style={{
                ...styles.cell,
                background: cell.isRevealed ? '#c0c0c0' : '#d4d4d4',
                border: cell.isRevealed
                  ? '1px solid #808080'
                  : '2px outset #ffffff',
                color: NUMBER_COLORS[cell.neighborMines],
              }}
            >
              {getCellContent(cell)}
            </div>
          ))
        )}
      </div>

      {(gameOver || won) && (
        <div style={{ ...styles.status, color: won ? 'green' : 'red' }}>
          {won ? 'You Win!' : 'Game Over!'}
        </div>
      )}
    </div>
  );
}

const styles = {
  container: {
    padding: '10px',
    background: '#c0c0c0',
    fontFamily: 'MS Sans Serif, Arial',
  },
  header: {
    display: 'flex',
    justifyContent: 'space-between',
    marginBottom: '10px',
    padding: '5px',
    background: '#808080',
    border: '2px inset #c0c0c0',
  },
  board: {
    display: 'grid',
    gridTemplateColumns: `repeat(${BOARD_SIZE}, 24px)`,
    gap: '1px',
    background: '#808080',
    padding: '2px',
    border: '3px inset #808080',
  },
  cell: {
    width: '24px',
    height: '24px',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    fontSize: '14px',
    fontWeight: 'bold',
    cursor: 'pointer',
    userSelect: 'none',
  },
  status: {
    textAlign: 'center',
    marginTop: '10px',
  },
};
