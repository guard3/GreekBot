SELECT ROW_NUMBER() OVER (ORDER BY xp DESC, num_msg DESC), id, xp, num_msg
FROM leaderboard
LIMIT 10;