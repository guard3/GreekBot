CREATE TABLE IF NOT EXISTS leaderboard(
    id INTEGER PRIMARY KEY,
    num_msg INTEGER NOT NULL,
    xp INTEGER NOT NULL,
    timestamp INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS welcoming(
    msg_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    joined_at INTEGER NOT NULL
);