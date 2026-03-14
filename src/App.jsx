import React, { useState, useEffect, useCallback, useRef } from 'react';

// ============ GAME COMPONENTS ============

// Snake Game
const SnakeGame = () => {
  const canvasRef = useRef(null);
  const [gameOver, setGameOver] = useState(false);
  const [score, setScore] = useState(0);
  const [gameStarted, setGameStarted] = useState(false);
  const gameState = useRef({
    snake: [{ x: 10, y: 10 }],
    food: { x: 15, y: 15 },
    direction: { x: 1, y: 0 },
    nextDirection: { x: 1, y: 0 }
  });

  const randomFood = (snake) => {
    let pos;
    do {
      pos = { x: Math.floor(Math.random() * 20), y: Math.floor(Math.random() * 20) };
    } while (snake.some(seg => seg.x === pos.x && seg.y === pos.y));
    return pos;
  };

  const resetGame = () => {
    const snake = [{ x: 10, y: 10 }];
    gameState.current = {
      snake,
      food: randomFood(snake),
      direction: { x: 1, y: 0 },
      nextDirection: { x: 1, y: 0 }
    };
    setScore(0);
    setGameOver(false);
    setGameStarted(true);
  };

  useEffect(() => {
    if (!gameStarted || gameOver) return;
    
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const gridSize = 15;
    
    const gameLoop = setInterval(() => {
      const state = gameState.current;
      state.direction = state.nextDirection;
      
      const newHead = {
        x: (state.snake[0].x + state.direction.x + 20) % 20,
        y: (state.snake[0].y + state.direction.y + 20) % 20
      };
      
      if (state.snake.some(seg => seg.x === newHead.x && seg.y === newHead.y)) {
        setGameOver(true);
        return;
      }
      
      state.snake.unshift(newHead);
      
      if (newHead.x === state.food.x && newHead.y === state.food.y) {
        setScore(s => s + 10);
        state.food = randomFood(state.snake);
      } else {
        state.snake.pop();
      }
      
      ctx.fillStyle = '#000080';
      ctx.fillRect(0, 0, 300, 300);
      
      ctx.fillStyle = '#00ff00';
      state.snake.forEach(seg => {
        ctx.fillRect(seg.x * gridSize + 1, seg.y * gridSize + 1, gridSize - 2, gridSize - 2);
      });
      
      ctx.fillStyle = '#ff0000';
      ctx.fillRect(state.food.x * gridSize + 2, state.food.y * gridSize + 2, gridSize - 4, gridSize - 4);
    }, 150);
    
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
        ArrowRight: { x: 1, y: 0 }
      };
      if (keyMap[e.key]) {
        const newDir = keyMap[e.key];
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
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', padding: '10px', background: '#c0c0c0' }}>
      <div style={{ marginBottom: '10px', fontFamily: 'MS Sans Serif, Arial', fontSize: '14px' }}>Score: {score}</div>
      <canvas ref={canvasRef} width={300} height={300} style={{ border: '2px inset #808080' }} />
      {(!gameStarted || gameOver) && (
        <div style={{ marginTop: '10px' }}>
          {gameOver && <div style={{ color: 'red', marginBottom: '5px' }}>Game Over!</div>}
          <button onClick={resetGame} style={win95Button}>
            {gameOver ? 'Play Again' : 'Start Game'}
          </button>
        </div>
      )}
      <div style={{ marginTop: '10px', fontSize: '12px', color: '#404040' }}>Use Arrow Keys to move</div>
    </div>
  );
};

// Minesweeper Game
const Minesweeper = () => {
  const [board, setBoard] = useState([]);
  const [gameOver, setGameOver] = useState(false);
  const [won, setWon] = useState(false);
  const [flagCount, setFlagCount] = useState(0);
  const size = 8;
  const mines = 10;

  const initBoard = useCallback(() => {
    const newBoard = Array(size).fill(null).map(() =>
      Array(size).fill(null).map(() => ({
        isMine: false,
        isRevealed: false,
        isFlagged: false,
        neighborMines: 0
      }))
    );
    
    let placed = 0;
    while (placed < mines) {
      const x = Math.floor(Math.random() * size);
      const y = Math.floor(Math.random() * size);
      if (!newBoard[y][x].isMine) {
        newBoard[y][x].isMine = true;
        placed++;
      }
    }
    
    for (let y = 0; y < size; y++) {
      for (let x = 0; x < size; x++) {
        if (!newBoard[y][x].isMine) {
          let count = 0;
          for (let dy = -1; dy <= 1; dy++) {
            for (let dx = -1; dx <= 1; dx++) {
              const ny = y + dy, nx = x + dx;
              if (ny >= 0 && ny < size && nx >= 0 && nx < size && newBoard[ny][nx].isMine) {
                count++;
              }
            }
          }
          newBoard[y][x].neighborMines = count;
        }
      }
    }
    
    setBoard(newBoard);
    setGameOver(false);
    setWon(false);
    setFlagCount(0);
  }, []);

  useEffect(() => { initBoard(); }, [initBoard]);

  const reveal = (y, x) => {
    if (gameOver || won || board[y][x].isRevealed || board[y][x].isFlagged) return;
    
    const newBoard = board.map(row => row.map(cell => ({ ...cell })));
    
    if (newBoard[y][x].isMine) {
      newBoard.forEach(row => row.forEach(cell => {
        if (cell.isMine) cell.isRevealed = true;
      }));
      setBoard(newBoard);
      setGameOver(true);
      return;
    }
    
    const flood = (cy, cx) => {
      if (cy < 0 || cy >= size || cx < 0 || cx >= size) return;
      if (newBoard[cy][cx].isRevealed || newBoard[cy][cx].isFlagged || newBoard[cy][cx].isMine) return;
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
    
    const unrevealed = newBoard.flat().filter(c => !c.isRevealed && !c.isMine).length;
    if (unrevealed === 0) setWon(true);
  };

  const toggleFlag = (e, y, x) => {
    e.preventDefault();
    if (gameOver || won || board[y][x].isRevealed) return;
    const newBoard = board.map(row => row.map(cell => ({ ...cell })));
    newBoard[y][x].isFlagged = !newBoard[y][x].isFlagged;
    setBoard(newBoard);
    setFlagCount(f => newBoard[y][x].isFlagged ? f + 1 : f - 1);
  };

  const colors = ['', '#0000ff', '#008000', '#ff0000', '#000080', '#800000', '#008080', '#000000', '#808080'];

  return (
    <div style={{ padding: '10px', background: '#c0c0c0', fontFamily: 'MS Sans Serif, Arial' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '10px', padding: '5px', background: '#808080', border: '2px inset #c0c0c0' }}>
        <span>💣 {mines - flagCount}</span>
        <span>{gameOver ? '😵' : won ? '😎' : '🙂'}</span>
        <button onClick={initBoard} style={{ ...win95Button, padding: '2px 8px' }}>New</button>
      </div>
      <div style={{ display: 'grid', gridTemplateColumns: `repeat(${size}, 24px)`, gap: '1px', background: '#808080', padding: '2px', border: '3px inset #808080' }}>
        {board.map((row, y) => row.map((cell, x) => (
          <div
            key={`${y}-${x}`}
            onClick={() => reveal(y, x)}
            onContextMenu={(e) => toggleFlag(e, y, x)}
            style={{
              width: '24px',
              height: '24px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              fontSize: '14px',
              fontWeight: 'bold',
              cursor: 'pointer',
              background: cell.isRevealed ? '#c0c0c0' : '#d4d4d4',
              border: cell.isRevealed ? '1px solid #808080' : '2px outset #ffffff',
              color: colors[cell.neighborMines],
              userSelect: 'none'
            }}
          >
            {cell.isFlagged ? '🚩' : cell.isRevealed ? (cell.isMine ? '💣' : (cell.neighborMines || '')) : ''}
          </div>
        )))}
      </div>
      {(gameOver || won) && (
        <div style={{ textAlign: 'center', marginTop: '10px', color: won ? 'green' : 'red' }}>
          {won ? 'You Win!' : 'Game Over!'}
        </div>
      )}
    </div>
  );
};

// Tetris Game
const TETRIS_SHAPES = [
  [[1,1,1,1]], // I
  [[1,1],[1,1]], // O
  [[0,1,0],[1,1,1]], // T
  [[1,0,0],[1,1,1]], // L
  [[0,0,1],[1,1,1]], // J
  [[0,1,1],[1,1,0]], // S
  [[1,1,0],[0,1,1]]  // Z
];
const TETRIS_COLORS = ['#00ffff', '#ffff00', '#aa00ff', '#ff8800', '#0000ff', '#00ff00', '#ff0000'];

const Tetris = () => {
  const canvasRef = useRef(null);
  const [score, setScore] = useState(0);
  const [gameOver, setGameOver] = useState(false);
  const [gameStarted, setGameStarted] = useState(false);
  const gameRef = useRef(null);

  const startGame = () => {
    const cols = 10, rows = 20;
    gameRef.current = {
      board: Array(rows).fill(null).map(() => Array(cols).fill(0)),
      piece: null,
      pieceX: 0,
      pieceY: 0,
      pieceColor: 0
    };
    setScore(0);
    setGameOver(false);
    setGameStarted(true);
    spawnPiece();
  };

  const spawnPiece = () => {
    const idx = Math.floor(Math.random() * TETRIS_SHAPES.length);
    gameRef.current.piece = TETRIS_SHAPES[idx].map(row => [...row]);
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
          if (newX < 0 || newX >= 10 || newY >= 20) return true;
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
    
    let cleared = 0;
    for (let y = 19; y >= 0; y--) {
      if (game.board[y].every(c => c)) {
        game.board.splice(y, 1);
        game.board.unshift(Array(10).fill(0));
        cleared++;
        y++;
      }
    }
    if (cleared) setScore(s => s + cleared * 100);
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
    const rotated = game.piece[0].map((_, i) => game.piece.map(row => row[i]).reverse());
    if (!collides(0, 0, rotated)) {
      game.piece = rotated;
    }
  };

  const drop = () => {
    if (!move(0, 1)) merge();
  };

  useEffect(() => {
    if (!gameStarted || gameOver) return;
    const interval = setInterval(drop, 500);
    return () => clearInterval(interval);
  }, [gameStarted, gameOver]);

  useEffect(() => {
    if (!gameStarted || gameOver) return;
    
    const draw = () => {
      const canvas = canvasRef.current;
      const ctx = canvas.getContext('2d');
      const game = gameRef.current;
      const cellSize = 20;
      
      ctx.fillStyle = '#000';
      ctx.fillRect(0, 0, 200, 400);
      
      for (let y = 0; y < 20; y++) {
        for (let x = 0; x < 10; x++) {
          if (game.board[y][x]) {
            ctx.fillStyle = TETRIS_COLORS[game.board[y][x] - 1];
            ctx.fillRect(x * cellSize + 1, y * cellSize + 1, cellSize - 2, cellSize - 2);
          }
        }
      }
      
      if (game.piece) {
        ctx.fillStyle = TETRIS_COLORS[game.pieceColor];
        for (let y = 0; y < game.piece.length; y++) {
          for (let x = 0; x < game.piece[y].length; x++) {
            if (game.piece[y][x]) {
              ctx.fillRect((game.pieceX + x) * cellSize + 1, (game.pieceY + y) * cellSize + 1, cellSize - 2, cellSize - 2);
            }
          }
        }
      }
    };
    
    const animFrame = setInterval(draw, 50);
    return () => clearInterval(animFrame);
  }, [gameStarted, gameOver]);

  useEffect(() => {
    if (!gameStarted) return;
    const handleKey = (e) => {
      if (e.key === 'ArrowLeft') move(-1, 0);
      if (e.key === 'ArrowRight') move(1, 0);
      if (e.key === 'ArrowDown') drop();
      if (e.key === 'ArrowUp') rotate();
      e.preventDefault();
    };
    window.addEventListener('keydown', handleKey);
    return () => window.removeEventListener('keydown', handleKey);
  }, [gameStarted]);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', padding: '10px', background: '#c0c0c0' }}>
      <div style={{ marginBottom: '10px', fontFamily: 'MS Sans Serif, Arial' }}>Score: {score}</div>
      <canvas ref={canvasRef} width={200} height={400} style={{ border: '3px inset #808080' }} />
      {(!gameStarted || gameOver) && (
        <div style={{ marginTop: '10px' }}>
          {gameOver && <div style={{ color: 'red', marginBottom: '5px' }}>Game Over!</div>}
          <button onClick={startGame} style={win95Button}>
            {gameOver ? 'Play Again' : 'Start Game'}
          </button>
        </div>
      )}
      <div style={{ marginTop: '10px', fontSize: '11px', color: '#404040', textAlign: 'center' }}>
        ← → Move | ↑ Rotate | ↓ Drop
      </div>
    </div>
  );
};

// Memory Card Game
const MemoryGame = () => {
  const emojis = ['🎮', '🎲', '🎯', '🎪', '🎨', '🎭', '🎵', '🎸'];
  const [cards, setCards] = useState([]);
  const [flipped, setFlipped] = useState([]);
  const [matched, setMatched] = useState([]);
  const [moves, setMoves] = useState(0);

  const initGame = useCallback(() => {
    const pairs = [...emojis, ...emojis];
    const shuffled = pairs.sort(() => Math.random() - 0.5);
    setCards(shuffled);
    setFlipped([]);
    setMatched([]);
    setMoves(0);
  }, []);

  useEffect(() => { initGame(); }, [initGame]);

  const handleClick = (index) => {
    if (flipped.length === 2 || flipped.includes(index) || matched.includes(index)) return;
    
    const newFlipped = [...flipped, index];
    setFlipped(newFlipped);
    
    if (newFlipped.length === 2) {
      setMoves(m => m + 1);
      if (cards[newFlipped[0]] === cards[newFlipped[1]]) {
        setMatched(m => [...m, ...newFlipped]);
        setFlipped([]);
      } else {
        setTimeout(() => setFlipped([]), 1000);
      }
    }
  };

  return (
    <div style={{ padding: '15px', background: '#c0c0c0', fontFamily: 'MS Sans Serif, Arial' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '10px' }}>
        <span>Moves: {moves}</span>
        <button onClick={initGame} style={win95Button}>New Game</button>
      </div>
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 60px)', gap: '8px' }}>
        {cards.map((emoji, i) => (
          <div
            key={i}
            onClick={() => handleClick(i)}
            style={{
              width: '60px',
              height: '60px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              fontSize: '28px',
              cursor: 'pointer',
              background: matched.includes(i) ? '#90EE90' : flipped.includes(i) ? '#fff' : '#008080',
              border: '3px outset #d4d4d4',
              borderRadius: '4px',
              transition: 'all 0.2s'
            }}
          >
            {(flipped.includes(i) || matched.includes(i)) ? emoji : '❓'}
          </div>
        ))}
      </div>
      {matched.length === cards.length && (
        <div style={{ textAlign: 'center', marginTop: '15px', color: 'green', fontWeight: 'bold' }}>
          🎉 You Won in {moves} moves!
        </div>
      )}
    </div>
  );
};

// Pong Game
const PongGame = () => {
  const canvasRef = useRef(null);
  const [score, setScore] = useState({ player: 0, cpu: 0 });
  const [gameStarted, setGameStarted] = useState(false);
  const gameState = useRef({
    playerY: 150,
    cpuY: 150,
    ballX: 200,
    ballY: 150,
    ballVX: 4,
    ballVY: 2
  });

  const startGame = () => {
    gameState.current = {
      playerY: 150,
      cpuY: 150,
      ballX: 200,
      ballY: 150,
      ballVX: 4 * (Math.random() > 0.5 ? 1 : -1),
      ballVY: 2 * (Math.random() > 0.5 ? 1 : -1)
    };
    setScore({ player: 0, cpu: 0 });
    setGameStarted(true);
  };

  useEffect(() => {
    if (!gameStarted) return;
    
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const paddleH = 60, paddleW = 10, ballR = 8;
    
    const gameLoop = setInterval(() => {
      const s = gameState.current;
      
      // Ball movement
      s.ballX += s.ballVX;
      s.ballY += s.ballVY;
      
      // Wall bounce
      if (s.ballY <= ballR || s.ballY >= 300 - ballR) s.ballVY *= -1;
      
      // Paddle collision
      const maxSpeed = 12;
      if (s.ballX <= 20 + paddleW && s.ballY > s.playerY && s.ballY < s.playerY + paddleH) {
        s.ballVX = Math.min(Math.abs(s.ballVX) * 1.05, maxSpeed);
        s.ballVY += (s.ballY - (s.playerY + paddleH/2)) * 0.1;
      }
      if (s.ballX >= 380 - paddleW && s.ballY > s.cpuY && s.ballY < s.cpuY + paddleH) {
        s.ballVX = -Math.min(Math.abs(s.ballVX) * 1.05, maxSpeed);
      }
      
      // Score
      if (s.ballX < 0) {
        setScore(sc => ({ ...sc, cpu: sc.cpu + 1 }));
        s.ballX = 200; s.ballY = 150; s.ballVX = 4; s.ballVY = 2;
      }
      if (s.ballX > 400) {
        setScore(sc => ({ ...sc, player: sc.player + 1 }));
        s.ballX = 200; s.ballY = 150; s.ballVX = -4; s.ballVY = 2;
      }
      
      // CPU AI
      const cpuCenter = s.cpuY + paddleH / 2;
      if (cpuCenter < s.ballY - 20) s.cpuY += 3;
      if (cpuCenter > s.ballY + 20) s.cpuY -= 3;
      s.cpuY = Math.max(0, Math.min(300 - paddleH, s.cpuY));
      
      // Draw
      ctx.fillStyle = '#000';
      ctx.fillRect(0, 0, 400, 300);
      
      ctx.strokeStyle = '#444';
      ctx.setLineDash([10, 10]);
      ctx.beginPath();
      ctx.moveTo(200, 0);
      ctx.lineTo(200, 300);
      ctx.stroke();
      ctx.setLineDash([]);
      
      ctx.fillStyle = '#0f0';
      ctx.fillRect(10, s.playerY, paddleW, paddleH);
      ctx.fillStyle = '#f00';
      ctx.fillRect(380, s.cpuY, paddleW, paddleH);
      
      ctx.fillStyle = '#fff';
      ctx.beginPath();
      ctx.arc(s.ballX, s.ballY, ballR, 0, Math.PI * 2);
      ctx.fill();
    }, 16);
    
    return () => clearInterval(gameLoop);
  }, [gameStarted]);

  useEffect(() => {
    if (!gameStarted) return;
    const handleMouse = (e) => {
      const canvas = canvasRef.current;
      const rect = canvas.getBoundingClientRect();
      gameState.current.playerY = Math.max(0, Math.min(240, e.clientY - rect.top - 30));
    };
    const canvas = canvasRef.current;
    canvas.addEventListener('mousemove', handleMouse);
    return () => canvas.removeEventListener('mousemove', handleMouse);
  }, [gameStarted]);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', padding: '10px', background: '#c0c0c0' }}>
      <div style={{ marginBottom: '10px', fontFamily: 'MS Sans Serif, Arial', display: 'flex', gap: '30px' }}>
        <span style={{ color: '#008000' }}>You: {score.player}</span>
        <span style={{ color: '#800000' }}>CPU: {score.cpu}</span>
      </div>
      <canvas ref={canvasRef} width={400} height={300} style={{ border: '3px inset #808080', cursor: 'none' }} />
      {!gameStarted && (
        <button onClick={startGame} style={{ ...win95Button, marginTop: '10px' }}>Start Game</button>
      )}
      <div style={{ marginTop: '10px', fontSize: '11px', color: '#404040' }}>Move mouse to control paddle</div>
    </div>
  );
};

// ============ STYLES ============

const win95Button = {
  padding: '4px 16px',
  background: '#c0c0c0',
  border: '2px outset #dfdfdf',
  fontFamily: 'MS Sans Serif, Arial',
  fontSize: '12px',
  cursor: 'pointer',
  outline: 'none'
};

// ============ MAIN APP ============

export default function RetroOS() {
  const [startMenuOpen, setStartMenuOpen] = useState(false);
  const [openWindows, setOpenWindows] = useState([]);
  const [activeWindow, setActiveWindow] = useState(null);
  const [time, setTime] = useState(new Date());

  useEffect(() => {
    const timer = setInterval(() => setTime(new Date()), 1000);
    return () => clearInterval(timer);
  }, []);

  const games = [
    { id: 'snake', name: 'Snake', icon: '🐍', component: SnakeGame },
    { id: 'minesweeper', name: 'Minesweeper', icon: '💣', component: Minesweeper },
    { id: 'tetris', name: 'Tetris', icon: '🧱', component: Tetris },
    { id: 'memory', name: 'Memory', icon: '🃏', component: MemoryGame },
    { id: 'pong', name: 'Pong', icon: '🏓', component: PongGame }
  ];

  const openGame = (game) => {
    if (!openWindows.find(w => w.id === game.id)) {
      setOpenWindows([...openWindows, { ...game, x: 50 + openWindows.length * 30, y: 50 + openWindows.length * 30 }]);
    }
    setActiveWindow(game.id);
    setStartMenuOpen(false);
  };

  const closeWindow = (id) => {
    setOpenWindows(openWindows.filter(w => w.id !== id));
    if (activeWindow === id) setActiveWindow(null);
  };

  const [dragging, setDragging] = useState(null);
  const [dragOffset, setDragOffset] = useState({ x: 0, y: 0 });

  const handleMouseDown = (e, windowId) => {
    const win = openWindows.find(w => w.id === windowId);
    setDragging(windowId);
    setDragOffset({ x: e.clientX - win.x, y: e.clientY - win.y });
    setActiveWindow(windowId);
  };

  const handleMouseMove = (e) => {
    if (!dragging) return;
    setOpenWindows(openWindows.map(w => 
      w.id === dragging ? { ...w, x: e.clientX - dragOffset.x, y: e.clientY - dragOffset.y } : w
    ));
  };

  const handleMouseUp = () => setDragging(null);

  return (
    <div 
      style={{
        width: '100vw',
        height: '100vh',
        background: '#008080',
        position: 'relative',
        overflow: 'hidden',
        fontFamily: '"MS Sans Serif", "Segoe UI", Tahoma, sans-serif',
        userSelect: 'none'
      }}
      onMouseMove={handleMouseMove}
      onMouseUp={handleMouseUp}
    >
      {/* Desktop Icons */}
      <div style={{ padding: '20px', display: 'flex', flexDirection: 'column', gap: '15px', position: 'absolute', top: 0, left: 0 }}>
        {games.map(game => (
          <div
            key={game.id}
            onDoubleClick={() => openGame(game)}
            style={{
              width: '75px',
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              cursor: 'pointer',
              padding: '5px',
              borderRadius: '2px'
            }}
            onMouseEnter={(e) => e.currentTarget.style.background = 'rgba(0,0,128,0.3)'}
            onMouseLeave={(e) => e.currentTarget.style.background = 'transparent'}
          >
            <div style={{ fontSize: '32px', textShadow: '2px 2px 2px rgba(0,0,0,0.3)' }}>{game.icon}</div>
            <span style={{ color: '#fff', fontSize: '11px', textAlign: 'center', textShadow: '1px 1px 1px #000', marginTop: '4px' }}>
              {game.name}
            </span>
          </div>
        ))}
      </div>

      {/* Windows */}
      {openWindows.map(win => {
        const GameComponent = win.component;
        return (
          <div
            key={win.id}
            style={{
              position: 'absolute',
              left: win.x,
              top: win.y,
              background: '#c0c0c0',
              border: '2px outset #dfdfdf',
              boxShadow: activeWindow === win.id ? '4px 4px 8px rgba(0,0,0,0.4)' : '2px 2px 4px rgba(0,0,0,0.2)',
              zIndex: activeWindow === win.id ? 100 : 10
            }}
            onClick={() => setActiveWindow(win.id)}
          >
            {/* Title Bar */}
            <div
              onMouseDown={(e) => handleMouseDown(e, win.id)}
              style={{
                background: activeWindow === win.id ? 'linear-gradient(90deg, #000080, #1084d0)' : 'linear-gradient(90deg, #808080, #a0a0a0)',
                padding: '3px 5px',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'space-between',
                cursor: 'move'
              }}
            >
              <span style={{ color: '#fff', fontSize: '12px', fontWeight: 'bold', display: 'flex', alignItems: 'center', gap: '5px' }}>
                <span style={{ fontSize: '14px' }}>{win.icon}</span>
                {win.name}
              </span>
              <div style={{ display: 'flex', gap: '2px' }}>
                <button
                  onClick={(e) => { e.stopPropagation(); closeWindow(win.id); }}
                  style={{
                    width: '20px',
                    height: '20px',
                    background: '#c0c0c0',
                    border: '2px outset #dfdfdf',
                    fontSize: '12px',
                    fontWeight: 'bold',
                    cursor: 'pointer',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                    lineHeight: 1
                  }}
                >
                  ✕
                </button>
              </div>
            </div>
            {/* Content */}
            <div style={{ border: '2px inset #808080' }}>
              <GameComponent />
            </div>
          </div>
        );
      })}

      {/* Start Menu */}
      {startMenuOpen && (
        <div style={{
          position: 'absolute',
          bottom: '40px',
          left: '0',
          width: '220px',
          background: '#c0c0c0',
          border: '2px outset #dfdfdf',
          boxShadow: '4px 4px 8px rgba(0,0,0,0.3)',
          zIndex: 1000
        }}>
          <div style={{
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
            justifyContent: 'center'
          }}>
            <span style={{ color: '#c0c0c0', fontSize: '20px', fontWeight: 'bold', letterSpacing: '2px' }}>
              RetroOS<span style={{ color: '#fff' }}>95</span>
            </span>
          </div>
          <div style={{ marginLeft: '30px', padding: '5px 0' }}>
            <div style={{ padding: '5px 10px', borderBottom: '1px solid #808080', fontSize: '12px', color: '#404040' }}>
              🎮 Games
            </div>
            {games.map(game => (
              <div
                key={game.id}
                onClick={() => openGame(game)}
                style={{
                  padding: '8px 15px',
                  display: 'flex',
                  alignItems: 'center',
                  gap: '10px',
                  cursor: 'pointer',
                  fontSize: '13px'
                }}
                onMouseEnter={(e) => e.currentTarget.style.background = '#000080'}
                onMouseLeave={(e) => e.currentTarget.style.background = 'transparent'}
                onMouseOver={(e) => e.currentTarget.style.color = '#fff'}
                onMouseOut={(e) => e.currentTarget.style.color = '#000'}
              >
                <span style={{ fontSize: '18px' }}>{game.icon}</span>
                {game.name}
              </div>
            ))}
            <div style={{ borderTop: '2px groove #dfdfdf', margin: '5px 0' }} />
            <div
              style={{ padding: '8px 15px', display: 'flex', alignItems: 'center', gap: '10px', cursor: 'pointer', fontSize: '13px' }}
              onMouseEnter={(e) => { e.currentTarget.style.background = '#000080'; e.currentTarget.style.color = '#fff'; }}
              onMouseLeave={(e) => { e.currentTarget.style.background = 'transparent'; e.currentTarget.style.color = '#000'; }}
            >
              <span style={{ fontSize: '18px' }}>🔌</span>
              Shut Down...
            </div>
          </div>
        </div>
      )}

      {/* Taskbar */}
      <div style={{
        position: 'absolute',
        bottom: 0,
        left: 0,
        right: 0,
        height: '36px',
        background: '#c0c0c0',
        borderTop: '2px outset #dfdfdf',
        display: 'flex',
        alignItems: 'center',
        padding: '0 4px',
        gap: '4px'
      }}>
        {/* Start Button */}
        <button
          onClick={() => setStartMenuOpen(!startMenuOpen)}
          style={{
            ...win95Button,
            padding: '4px 10px',
            display: 'flex',
            alignItems: 'center',
            gap: '5px',
            fontWeight: 'bold',
            border: startMenuOpen ? '2px inset #808080' : '2px outset #dfdfdf'
          }}
        >
          <span style={{ fontSize: '16px' }}>🪟</span>
          Start
        </button>

        {/* Taskbar separator */}
        <div style={{ width: '2px', height: '24px', background: '#808080', marginLeft: '4px' }} />

        {/* Open Windows */}
        <div style={{ flex: 1, display: 'flex', gap: '4px', overflow: 'hidden' }}>
          {openWindows.map(win => (
            <button
              key={win.id}
              onClick={() => setActiveWindow(win.id)}
              style={{
                ...win95Button,
                minWidth: '120px',
                maxWidth: '160px',
                display: 'flex',
                alignItems: 'center',
                gap: '5px',
                textAlign: 'left',
                overflow: 'hidden',
                border: activeWindow === win.id ? '2px inset #808080' : '2px outset #dfdfdf',
                background: activeWindow === win.id ? '#a0a0a0' : '#c0c0c0'
              }}
            >
              <span style={{ fontSize: '14px' }}>{win.icon}</span>
              <span style={{ overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>{win.name}</span>
            </button>
          ))}
        </div>

        {/* System Tray */}
        <div style={{
          padding: '2px 8px',
          border: '2px inset #808080',
          background: '#c0c0c0',
          fontSize: '12px',
          display: 'flex',
          alignItems: 'center',
          gap: '8px'
        }}>
          <span>🔊</span>
          <span>{time.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}</span>
        </div>
      </div>
      
      {/* Click outside to close start menu */}
      {startMenuOpen && (
        <div 
          style={{ position: 'fixed', top: 0, left: 0, right: 0, bottom: '36px', zIndex: 999 }}
          onClick={() => setStartMenuOpen(false)}
        />
      )}
    </div>
  );
}
