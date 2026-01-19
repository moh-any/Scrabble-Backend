-- +goose Up
-- +goose StatementBegin
CREATE TABLE IF NOT EXISTS games (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    player1_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    player2_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    winner_id UUID REFERENCES users(id) ON DELETE SET NULL,
    end_reason VARCHAR(50),
    player1_score INTEGER NOT NULL DEFAULT 0,
    player2_score INTEGER NOT NULL DEFAULT 0,
    started_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    ended_at TIMESTAMP WITH TIME ZONE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE OR REPLACE FUNCTION update_user_stats()
RETURNS TRIGGER AS $$
BEGIN
    IF NEW.ended_at IS NOT NULL THEN
        UPDATE users 
        SET total_games = total_games + 1
        WHERE id IN (NEW.player1_id, NEW.player2_id);
        
        IF NEW.winner_id IS NOT NULL THEN
            UPDATE users 
            SET wins = wins + 1
            WHERE id = NEW.winner_id;

            UPDATE users 
            SET losses = losses + 1
            WHERE id IN (NEW.player1_id, NEW.player2_id) 
            AND id != NEW.winner_id;
        ELSE
            UPDATE users 
            SET draws = draws + 1
            WHERE id IN (NEW.player1_id, NEW.player2_id);
        END IF;
    END IF;
    
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER user_stats_trigger
AFTER INSERT OR UPDATE ON games
FOR EACH ROW
EXECUTE FUNCTION update_user_stats();
-- +goose StatementEnd

-- +goose Down
-- +goose StatementBegin
DROP TRIGGER IF EXISTS user_stats_trigger ON games;
DROP FUNCTION IF EXISTS update_user_stats();
DROP TABLE IF EXISTS games;
-- +goose StatementEnd