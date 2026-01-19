package store

import (
	"database/sql"
	"time"

	"github.com/google/uuid"
)

type Game struct {
	ID           uuid.UUID  `json:"id"`
	Player1ID    uuid.UUID  `json:"player1_id"`
	Player2ID    uuid.UUID  `json:"player2_id"`
	WinnerID     *uuid.UUID `json:"winner_id"`
	EndReason    *string    `json:"end_reason"`
	Player1Score int        `json:"player1_score"`
	Player2Score int        `json:"player2_score"`
	StartedAt    time.Time  `json:"started_at"`
	EndedAt      *time.Time `json:"ended_at"`
	CreatedAt    time.Time  `json:"created_at"`
}

type PostgresGameStore struct {
	db *sql.DB
}

func NewPostgresGameStore(db *sql.DB) *PostgresGameStore {
	return &PostgresGameStore{db: db}
}

type GameStore interface {
	CreateGame(game *Game) error
	GetGameByID(id uuid.UUID) (*Game, error)
	GetGamesByUserID(userID uuid.UUID) ([]*Game, error) // to add pagination later
}

func (s *PostgresGameStore) CreateGame(game *Game) error {
	query := `
        INSERT INTO games (player1_id, player2_id, winner_id, end_reason, player1_score, player2_score, started_at, ended_at)
        VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
        RETURNING id, created_at
    `
	return s.db.QueryRow(query,
		game.Player1ID,
		game.Player2ID,
		game.WinnerID,
		game.EndReason,
		game.Player1Score,
		game.Player2Score,
		game.StartedAt,
		game.EndedAt,
	).Scan(&game.ID, &game.CreatedAt)
}

func (s *PostgresGameStore) GetGameByID(id uuid.UUID) (*Game, error) {
	query := `
        SELECT id, player1_id, player2_id, winner_id, end_reason, player1_score, player2_score, started_at, ended_at, created_at
        FROM games
        WHERE id = $1
    `
	game := &Game{}
	err := s.db.QueryRow(query, id).Scan(
		&game.ID,
		&game.Player1ID,
		&game.Player2ID,
		&game.WinnerID,
		&game.EndReason,
		&game.Player1Score,
		&game.Player2Score,
		&game.StartedAt,
		&game.EndedAt,
		&game.CreatedAt,
	)
	return game, err
}

func (s *PostgresGameStore) GetGamesByUserID(userID uuid.UUID) ([]*Game, error) {
	query := `
        SELECT id, player1_id, player2_id, winner_id, end_reason, player1_score, player2_score, started_at, ended_at, created_at
        FROM games
        WHERE player1_id = $1 OR player2_id = $1
        ORDER BY created_at DESC
    `
	rows, err := s.db.Query(query, userID)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var games []*Game
	for rows.Next() {
		game := &Game{}
		err := rows.Scan(
			&game.ID,
			&game.Player1ID,
			&game.Player2ID,
			&game.WinnerID,
			&game.EndReason,
			&game.Player1Score,
			&game.Player2Score,
			&game.StartedAt,
			&game.EndedAt,
			&game.CreatedAt,
		)
		if err != nil {
			return nil, err
		}
		games = append(games, game)
	}
	return games, rows.Err()
}
