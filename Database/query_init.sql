CREATE TABLE IF NOT EXISTS leaderboard(
    id INTEGER PRIMARY KEY,
    num_msg INTEGER NOT NULL,
    xp INTEGER NOT NULL,
    timestamp INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS welcoming(
    user_id INTEGER PRIMARY KEY,
    msg_id INTEGER UNIQUE,
    old_id INTEGER UNIQUE,
    joined_at INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS starboard(
    msg_id INTEGER PRIMARY KEY,
    author_id INTEGER NOT NULL,
    sb_msg_id INTEGER UNIQUE,
    num INTEGER NOT NULL CHECK(num > 0)
);
CREATE TABLE IF NOT EXISTS messages(
    id INTEGER PRIMARY KEY,
    channel_id INTEGER NOT NULL,
    author_id INTEGER NOT NULL,
    content VARCHAR
);
CREATE TABLE IF NOT EXISTS tempbans(
    user_id INTEGER PRIMARY KEY,
    expires_at INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS infractions(
    user_id INTEGER NOT NULL,
    timestamp INTEGER NOT NULL,
    timed_out INTEGER NOT NULL,
    reason VARCHAR,

    PRIMARY KEY (user_id, timestamp)
);