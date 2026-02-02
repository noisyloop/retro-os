import React, { useState, useEffect, useRef } from 'react';
import { WIN95_BUTTON } from '../../../constants';

const CANVAS_WIDTH = 400;
const CANVAS_HEIGHT = 300;
const PADDLE_HEIGHT = 60;
const PADDLE_WIDTH = 10;
const BALL_RADIUS = 8;
const BALL_SPEED = 4;
const CPU_SPEED = 3;

export default function Pong() {
  const canvasRef = useRef(null);
  const [score, setScore] = useState({ player: 0, cpu: 0 });
  const [gameStarted, setGameStarted] = useState(false);

  const gameState = useRef({
    playerY: CANVAS_HEIGHT / 2 - PADDLE_HEIGHT / 2,
    cpuY: CANVAS_HEIGHT / 2 - PADDLE_HEIGHT / 2,
    ballX: CANVAS_WIDTH / 2,
    ballY: CANVAS_HEIGHT / 2,
    ballVX: BALL_SPEED,
    ballVY: 2,
  });

  const resetBall = (direction = 1) => {
    const state = gameState.current;
    state.ballX = CANVAS_WIDTH / 2;
    state.ballY = CANVAS_HEIGHT / 2;
    state.ballVX = BALL_SPEED * direction;
    state.ballVY = 2 * (Math.random() > 0.5 ? 1 : -1);
  };

  const startGame = () => {
    gameState.current = {
      playerY: CANVAS_HEIGHT / 2 - PADDLE_HEIGHT / 2,
      cpuY: CANVAS_HEIGHT / 2 - PADDLE_HEIGHT / 2,
      ballX: CANVAS_WIDTH / 2,
      ballY: CANVAS_HEIGHT / 2,
      ballVX: BALL_SPEED * (Math.random() > 0.5 ? 1 : -1),
      ballVY: 2 * (Math.random() > 0.5 ? 1 : -1),
    };
    setScore({ player: 0, cpu: 0 });
    setGameStarted(true);
  };

  useEffect(() => {
    if (!gameStarted) return;

    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');

    const gameLoop = setInterval(() => {
      const s = gameState.current;

      // Ball movement
      s.ballX += s.ballVX;
      s.ballY += s.ballVY;

      // Wall bounce (top/bottom)
      if (s.ballY <= BALL_RADIUS || s.ballY >= CANVAS_HEIGHT - BALL_RADIUS) {
        s.ballVY *= -1;
      }

      // Player paddle collision
      if (
        s.ballX <= 20 + PADDLE_WIDTH &&
        s.ballY > s.playerY &&
        s.ballY < s.playerY + PADDLE_HEIGHT
      ) {
        s.ballVX = Math.abs(s.ballVX) * 1.05;
        s.ballVY += (s.ballY - (s.playerY + PADDLE_HEIGHT / 2)) * 0.1;
      }

      // CPU paddle collision
      if (
        s.ballX >= CANVAS_WIDTH - 20 - PADDLE_WIDTH &&
        s.ballY > s.cpuY &&
        s.ballY < s.cpuY + PADDLE_HEIGHT
      ) {
        s.ballVX = -Math.abs(s.ballVX) * 1.05;
      }

      // Scoring
      if (s.ballX < 0) {
        setScore((sc) => ({ ...sc, cpu: sc.cpu + 1 }));
        resetBall(1);
      }

      if (s.ballX > CANVAS_WIDTH) {
        setScore((sc) => ({ ...sc, player: sc.player + 1 }));
        resetBall(-1);
      }

      // CPU AI
      const cpuCenter = s.cpuY + PADDLE_HEIGHT / 2;
      if (cpuCenter < s.ballY - 20) {
        s.cpuY += CPU_SPEED;
      }
      if (cpuCenter > s.ballY + 20) {
        s.cpuY -= CPU_SPEED;
      }
      s.cpuY = Math.max(0, Math.min(CANVAS_HEIGHT - PADDLE_HEIGHT, s.cpuY));

      // Draw
      ctx.fillStyle = '#000';
      ctx.fillRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);

      // Center line
      ctx.strokeStyle = '#444';
      ctx.setLineDash([10, 10]);
      ctx.beginPath();
      ctx.moveTo(CANVAS_WIDTH / 2, 0);
      ctx.lineTo(CANVAS_WIDTH / 2, CANVAS_HEIGHT);
      ctx.stroke();
      ctx.setLineDash([]);

      // Player paddle (green)
      ctx.fillStyle = '#0f0';
      ctx.fillRect(10, s.playerY, PADDLE_WIDTH, PADDLE_HEIGHT);

      // CPU paddle (red)
      ctx.fillStyle = '#f00';
      ctx.fillRect(CANVAS_WIDTH - 20, s.cpuY, PADDLE_WIDTH, PADDLE_HEIGHT);

      // Ball
      ctx.fillStyle = '#fff';
      ctx.beginPath();
      ctx.arc(s.ballX, s.ballY, BALL_RADIUS, 0, Math.PI * 2);
      ctx.fill();
    }, 16);

    return () => clearInterval(gameLoop);
  }, [gameStarted]);

  // Mouse controls
  useEffect(() => {
    if (!gameStarted) return;

    const handleMouse = (e) => {
      const canvas = canvasRef.current;
      const rect = canvas.getBoundingClientRect();
      const mouseY = e.clientY - rect.top - PADDLE_HEIGHT / 2;
      gameState.current.playerY = Math.max(
        0,
        Math.min(CANVAS_HEIGHT - PADDLE_HEIGHT, mouseY)
      );
    };

    const canvas = canvasRef.current;
    canvas.addEventListener('mousemove', handleMouse);
    return () => canvas.removeEventListener('mousemove', handleMouse);
  }, [gameStarted]);

  return (
    <div style={styles.container}>
      <div style={styles.scoreBoard}>
        <span style={styles.playerScore}>You: {score.player}</span>
        <span style={styles.cpuScore}>CPU: {score.cpu}</span>
      </div>

      <canvas
        ref={canvasRef}
        width={CANVAS_WIDTH}
        height={CANVAS_HEIGHT}
        style={styles.canvas}
      />

      {!gameStarted && (
        <button onClick={startGame} style={{ ...WIN95_BUTTON, marginTop: '10px' }}>
          Start Game
        </button>
      )}

      <div style={styles.instructions}>Move mouse to control paddle</div>
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
  scoreBoard: {
    marginBottom: '10px',
    fontFamily: 'MS Sans Serif, Arial',
    display: 'flex',
    gap: '30px',
  },
  playerScore: {
    color: '#008000',
  },
  cpuScore: {
    color: '#800000',
  },
  canvas: {
    border: '3px inset #808080',
    cursor: 'none',
  },
  instructions: {
    marginTop: '10px',
    fontSize: '11px',
    color: '#404040',
  },
};
