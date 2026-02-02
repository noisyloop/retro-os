import React, { useState, useEffect, useCallback } from 'react';
import { WIN95_BUTTON } from '../../../constants';

const EMOJIS = ['🎮', '🎲', '🎯', '🎪', '🎨', '🎭', '🎪', '🎸'];
const FLIP_DELAY = 1000;

export default function Memory() {
  const [cards, setCards] = useState([]);
  const [flipped, setFlipped] = useState([]);
  const [matched, setMatched] = useState([]);
  const [moves, setMoves] = useState(0);

  const initGame = useCallback(() => {
    const pairs = [...EMOJIS, ...EMOJIS];
    const shuffled = pairs.sort(() => Math.random() - 0.5);
    setCards(shuffled);
    setFlipped([]);
    setMatched([]);
    setMoves(0);
  }, []);

  useEffect(() => {
    initGame();
  }, [initGame]);

  const handleClick = (index) => {
    // Ignore if already flipped, matched, or two cards showing
    if (
      flipped.length === 2 ||
      flipped.includes(index) ||
      matched.includes(index)
    ) {
      return;
    }

    const newFlipped = [...flipped, index];
    setFlipped(newFlipped);

    if (newFlipped.length === 2) {
      setMoves((m) => m + 1);

      // Check for match
      if (cards[newFlipped[0]] === cards[newFlipped[1]]) {
        setMatched((m) => [...m, ...newFlipped]);
        setFlipped([]);
      } else {
        // Hide cards after delay
        setTimeout(() => setFlipped([]), FLIP_DELAY);
      }
    }
  };

  const isCardVisible = (index) => {
    return flipped.includes(index) || matched.includes(index);
  };

  const getCardStyle = (index) => {
    const isMatched = matched.includes(index);
    const isFlipped = flipped.includes(index);

    return {
      ...styles.card,
      background: isMatched ? '#90EE90' : isFlipped ? '#fff' : '#008080',
    };
  };

  const hasWon = matched.length === cards.length && cards.length > 0;

  return (
    <div style={styles.container}>
      <div style={styles.header}>
        <span>Moves: {moves}</span>
        <button onClick={initGame} style={WIN95_BUTTON}>
          New Game
        </button>
      </div>

      <div style={styles.grid}>
        {cards.map((emoji, i) => (
          <div
            key={i}
            onClick={() => handleClick(i)}
            style={getCardStyle(i)}
          >
            {isCardVisible(i) ? emoji : '❓'}
          </div>
        ))}
      </div>

      {hasWon && (
        <div style={styles.winMessage}>🎉 You Won in {moves} moves!</div>
      )}
    </div>
  );
}

const styles = {
  container: {
    padding: '15px',
    background: '#c0c0c0',
    fontFamily: 'MS Sans Serif, Arial',
  },
  header: {
    display: 'flex',
    justifyContent: 'space-between',
    marginBottom: '10px',
  },
  grid: {
    display: 'grid',
    gridTemplateColumns: 'repeat(4, 60px)',
    gap: '8px',
  },
  card: {
    width: '60px',
    height: '60px',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    fontSize: '28px',
    cursor: 'pointer',
    border: '3px outset #d4d4d4',
    borderRadius: '4px',
    transition: 'all 0.2s',
  },
  winMessage: {
    textAlign: 'center',
    marginTop: '15px',
    color: 'green',
    fontWeight: 'bold',
  },
};
